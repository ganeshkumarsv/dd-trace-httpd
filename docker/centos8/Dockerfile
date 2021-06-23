FROM centos:8 AS build
RUN yum -y install git zlib-devel wget make cmake gcc-c++ httpd-devel redhat-rpm-config libarchive
RUN mkdir -p /var/tmp/localbuild/dd-opentracing-cpp /var/tmp/localbuild/dd-trace-httpd/ddtrace
WORKDIR /var/tmp/localbuild
RUN cd /var/tmp/localbuild && wget https://github.com/DataDog/dd-opentracing-cpp/archive/b7c44dbd942bb064dac75b66d76b5d97f79a3f3c.tar.gz && tar zxf b7c44dbd942bb064dac75b66d76b5d97f79a3f3c.tar.gz -C dd-opentracing-cpp --strip-components=1 && cd dd-opentracing-cpp && ./scripts/install_dependencies.sh && mkdir .build && cd .build && cmake -DBUILD_SHARED=Off -DBUILD_STATIC=On .. && make && make install
ADD ddtrace/ddtrace.h ddtrace/ddtrace.cpp ddtrace/mod_ddtrace.c ddtrace/Makefile ddtrace/modules.mk ddtrace/.deps /var/tmp/localbuild/dd-trace-httpd/ddtrace/
WORKDIR /var/tmp/localbuild/dd-trace-httpd/ddtrace
RUN make && g++ -shared -fPIC -DPIC .libs/mod_ddtrace.o .libs/ddtrace.o -L../../dd-opentracing-cpp/.build -L../../dd-opentracing-cpp/deps/lib -Wl,-Bstatic -ldd_opentracing -lopentracing -lcurl -lz -Wl,-Bdynamic -lpthread -lrt -O2 -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z -Wl,relro -Wl,-z -Wl,now -Wl,-soname -Wl,mod_ddtrace.so -o .libs/mod_ddtrace.so

FROM centos:8
RUN yum -y install httpd
COPY --from=build /var/tmp/localbuild/dd-trace-httpd/ddtrace/.libs/mod_ddtrace.so /usr/lib64/httpd/modules/mod_ddtrace.so
COPY docker/centos8/00-ddtrace.conf /etc/httpd/conf.modules.d/
COPY docker/centos8/run-httpd.sh /
RUN cp /usr/share/testpage/index.html /var/www/html/index.html
CMD ["/run-httpd.sh"]
