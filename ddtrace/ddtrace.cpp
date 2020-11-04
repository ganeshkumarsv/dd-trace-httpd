#include <datadog/opentracing.h>
#include <datadog/tags.h>
#include <opentracing/ext/tags.h>

#include "ddtrace.h"
#include "http_log.h"

APLOG_USE_MODULE(ddtrace);

namespace {

std::shared_ptr<opentracing::Tracer> tracer;
std::unordered_map<request_rec*, std::unique_ptr<opentracing::Span>> active_spans;

struct HeadersReader : opentracing::TextMapReader {
  HeadersReader(const apr_table_t *h) : headers(h) {}

  opentracing::expected<opentracing::string_view> LookupKey(opentracing::string_view key) const override {
    const char *value = apr_table_get(headers, key.data());
    if (value != NULL) {
      return opentracing::string_view{value};
    }
    return opentracing::make_unexpected(opentracing::key_not_found_error);
  }

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key, opentracing::string_view value)> f)
    const override {
      const apr_array_header_t *arr = apr_table_elts(headers);
      const apr_table_entry_t *elts = reinterpret_cast<const apr_table_entry_t *>(arr->elts);
      for (int i = 0; i < arr->nelts; i++) {
        auto result = f(elts[i].key, elts[i].val);
        if (!result) {
          return result;
        }
      }
      return {};
    }

  const apr_table_t *headers = NULL;
};

struct HeadersWriter : opentracing::TextMapWriter {
  HeadersWriter(apr_table_t *h) : headers(h) {}

  opentracing::expected<void> Set(opentracing::string_view key, opentracing::string_view value) const override {
    apr_table_set(headers, key.data(), value.data());
    return {};
  }

  apr_table_t *headers = NULL;
};

} //  namespace

int apr_table_len(const apr_table_t *h) {
  const apr_array_header_t *arr = apr_table_elts(h);
  return arr->nelts;
}

void ddtrace_create_tracer() {
  datadog::opentracing::TracerOptions options;
  options.service = "hacked-up-apache-module";
  options.operation_name_override = "apache.request";
  ap_version_t httpd_version;
  ap_get_server_revision(&httpd_version);
  options.version = "httpd " + std::to_string(httpd_version.major) + "." + std::to_string(httpd_version.minor) + "." + std::to_string(httpd_version.patch) + httpd_version.add_string;
  tracer = datadog::opentracing::makeTracer(options);
}
     
void ddtrace_start_span(request_rec *r) {
  opentracing::StartSpanOptions options;
  std::unique_ptr<opentracing::SpanContext> propagated_context;

  if (r->main != NULL) {
    // we should have an active span for the main request to "parent" off of
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef, &active_spans[r]->context());
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "ddtrace_start_span: child of main request");
  } else {
    // use a propagated context if available;
    auto span_context_maybe = tracer->Extract(HeadersReader{r->headers_in});
    if (span_context_maybe) {
      propagated_context = std::move(*span_context_maybe);
      options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef, propagated_context.get());
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "ddtrace_start_span: propagated context");
    }
  }
  options.tags.emplace_back(datadog::tags::span_type, "web");
  options.tags.emplace_back(opentracing::ext::http_url, r->uri);
  options.tags.emplace_back(opentracing::ext::http_method, r->method);
  options.tags.emplace_back(opentracing::ext::peer_address, r->useragent_ip);

  // need to filter "the_request" to remove query parameters.
  auto span = tracer->StartSpanWithOptions(r->the_request, options);
  auto user_agent = apr_table_get(r->headers_in, "User-Agent");
  if (user_agent != NULL) {
    span->SetTag("http.user_agent", user_agent);
  }
  if (r->ap_auth_type != NULL) {
    span->SetTag("http.auth_type", r->ap_auth_type);
  }
  span->SetTag("httpd.server_config", std::string(r->server->defn_name) + ":" + std::to_string(r->server->defn_line_number));
  active_spans[r] = std::move(span);
  ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "ddtrace_start_span: active_spans = %lu", active_spans.size());
  tracer->Inject(active_spans[r]->context(), HeadersWriter{r->headers_in});
}

void ddtrace_finish_span(request_rec *r) {
  if (active_spans.find(r) == active_spans.end()) {
    // If a request is corrupted (ie: 400 Bad Request), we don't get told to start handling a request,
    // but do get told when the request finishes. Start a span if we didn't have one already.
    ddtrace_start_span(r);
  }
  if (r->log_id != NULL) {
    active_spans[r]->SetTag("httpd.log_id", r->log_id);
  }
  if (r->handler != NULL) {
    active_spans[r]->SetTag("httpd.handler", r->handler);
  }
  if (r->filename != NULL) {
    active_spans[r]->SetTag("httpd.filename", r->filename);
  }
  if (r->hostname != NULL) {
    active_spans[r]->SetTag("httpd.hostname", r->hostname);
  }
  active_spans[r]->SetTag(opentracing::ext::http_status_code, std::to_string(r->status));
  if (r->status >= 500) {
    active_spans[r]->SetTag(opentracing::ext::error, true);
  }
  active_spans[r]->Finish();
  active_spans.erase(r);
  ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "ddtrace_finish_span: active_spans = %lu", active_spans.size());
}

