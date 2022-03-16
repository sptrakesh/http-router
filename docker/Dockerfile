FROM sptrakesh/cppbase as base

COPY CMakeLists.txt /opt/spt/http-router/CMakeLists.txt
COPY src /opt/spt/http-router/src

WORKDIR /opt/spt/http-router/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. \
  && make install

FROM sptrakesh/cppruntime

COPY --from=base /opt/spt/include /opt/spt/include
WORKDIR /opt/spt