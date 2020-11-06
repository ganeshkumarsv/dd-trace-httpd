FROM centos:6 AS build
RUN yum -y install centos-release-scl epel-release git zlib-devel wget
RUN yum -y install msgpack-devel devtoolset-9 cmake3 httpd24-httpd-devel
RUN ln -s /usr/bin/cmake3 /usr/bin/cmake
# set env to the same effect as `scl enable devtoolset-9 bash`
ENV MANPATH=/opt/rh/devtoolset-9/root/usr/share/man: \
    X_SCLS=devtoolset-9 \
    PCP_DIR=/opt/rh/devtoolset-9/root \
    LD_LIBRARY_PATH=/opt/rh/devtoolset-9/root/usr/lib64:/opt/rh/devtoolset-9/root/usr/lib:/opt/rh/devtoolset-9/root/usr/lib64/dyninst:/opt/rh/devtoolset-9/root/usr/lib/dyninst:/opt/rh/devtoolset-9/root/usr/lib64:/opt/rh/devtoolset-9/root/usr/lib \
    PATH=/opt/rh/devtoolset-9/root/usr/bin:/opt/rh/httpd24/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    PKG_CONFIG_PATH=/opt/rh/devtoolset-9/root/usr/lib64/pkgconfig \
    INFOPATH=/opt/rh/devtoolset-9/root/usr/share/info
RUN mkdir -p /var/tmp/localbuild/dd-trace-httpd/ddtrace
WORKDIR /var/tmp/localbuild
RUN cd /var/tmp/localbuild && git clone -b cgilmour/pic https://github.com/DataDog/dd-opentracing-cpp && cd dd-opentracing-cpp && ./scripts/install_dependencies.sh && mkdir .build && cd .build && cmake -DBUILD_SHARED=Off -DBUILD_STATIC=On .. && make && make install
ADD ddtrace/ddtrace.h ddtrace/ddtrace.cpp ddtrace/mod_ddtrace.c ddtrace/Makefile ddtrace/modules.mk ddtrace/.deps /var/tmp/localbuild/dd-trace-httpd/ddtrace/
WORKDIR /var/tmp/localbuild/dd-trace-httpd/ddtrace
RUN make && g++ -shared -fPIC -DPIC .libs/mod_ddtrace.o .libs/ddtrace.o -L../../dd-opentracing-cpp/.build -L../../dd-opentracing-cpp/deps/lib -Wl,-Bstatic -ldd_opentracing -lopentracing -lcurl -lz -Wl,-Bdynamic -lpthread -lrt -O2 -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z -Wl,relro -Wl,-z -Wl,now -Wl,-soname -Wl,mod_ddtrace.so -o .libs/mod_ddtrace.so

FROM centos:6
RUN yum -y install centos-release-scl
RUN yum -y install httpd24
COPY --from=build /var/tmp/localbuild/dd-trace-httpd/ddtrace/.libs/mod_ddtrace.so /opt/rh/httpd24/root/usr/lib64/httpd/modules/mod_ddtrace.so
COPY docker/centos6/00-ddtrace.conf /opt/rh/httpd24/root/etc/httpd/conf.modules.d/
RUN printf "%s\n" "export DD_SERVICE=traced-httpd-centos6" "export DD_AGENT_HOST=dd-agent" >> /opt/rh/httpd24/root/etc/sysconfig/httpd
RUN cp /opt/rh/httpd24/root/usr/share/httpd/noindex/index.html /opt/rh/httpd24/root/var/www/html/
CMD ["/opt/rh/httpd24/root/usr/sbin/apachectl", "-D", "FOREGROUND"]
