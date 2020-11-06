FROM ubuntu:18.04 AS build
RUN apt-get update && apt-get -y install build-essential apache2-dev cmake git wget
RUN mkdir -p /var/tmp/localbuild/dd-trace-httpd/ddtrace
WORKDIR /var/tmp/localbuild
RUN cd /var/tmp/localbuild && git clone -b cgilmour/pic https://github.com/DataDog/dd-opentracing-cpp && cd dd-opentracing-cpp && ./scripts/install_dependencies.sh && mkdir .build && cd .build && cmake -DBUILD_SHARED=Off -DBUILD_STATIC=On .. && make && make install
ADD ddtrace/ddtrace.h ddtrace/ddtrace.cpp ddtrace/mod_ddtrace.c ddtrace/Makefile ddtrace/modules.mk ddtrace/.deps /var/tmp/localbuild/dd-trace-httpd/ddtrace/
WORKDIR /var/tmp/localbuild/dd-trace-httpd/ddtrace
RUN make && g++ -shared -fPIC -DPIC .libs/mod_ddtrace.o .libs/ddtrace.o -L../../dd-opentracing-cpp/.build -L../../dd-opentracing-cpp/deps/lib -Wl,-Bstatic -ldd_opentracing -lopentracing -lcurl -lz -Wl,-Bdynamic -lpthread -O2 -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z -Wl,relro -Wl,-z -Wl,now -Wl,-soname -Wl,mod_ddtrace.so -o .libs/mod_ddtrace.so

FROM ubuntu:18.04
RUN apt-get update && apt-get -y install apache2
COPY --from=build /var/tmp/localbuild/dd-trace-httpd/ddtrace/.libs/mod_ddtrace.so /usr/lib/apache2/modules/mod_ddtrace.so
COPY docker/ubuntu1804/ddtrace.load /etc/apache2/mods-available/
RUN a2enmod ddtrace
RUN printf "%s\n" "export DD_SERVICE=traced-httpd-ubuntu1804" "export DD_AGENT_HOST=dd-agent" >> /etc/apache2/envvars
CMD ["apachectl", "-D", "FOREGROUND"]
