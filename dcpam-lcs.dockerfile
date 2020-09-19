FROM ubuntu:18.04

RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "build-essential"\
 "clang"

COPY conf/lcs_config.json /dcpam/conf/
COPY src/ /dcpam/src/
COPY makefile.lcs /dcpam/
WORKDIR /dcpam/
RUN make -f makefile.lcs
RUN rm *.o