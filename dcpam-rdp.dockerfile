FROM ubuntu:18.04

RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "build-essential"\
 "clang"\
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

COPY conf/rdp_config.json /dcpam/conf/
COPY src/ /dcpam/src/
COPY makefile.rdp /dcpam/
WORKDIR /dcpam/
RUN make -f makefile.rdp
RUN rm ./*.o
