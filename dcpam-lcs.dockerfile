FROM ubuntu:18.04 as build-env

RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "make"\
 "clang"\
 "libssl-dev"\
 && apt-get clean\
 && rm -rf /var/lib/apt/lists/*

COPY src/ /opt/dcpam/src/
COPY makefile.lcs /opt/dcpam/
WORKDIR /opt/dcpam/
RUN make -f makefile.lcs
RUN rm ./*.o && rm makefile.lcs && rm ./src -rf

FROM ubuntu:18.04
RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "libssl-dev"\
 && apt-get clean\
 && rm -rf /var/lib/apt/lists/*

COPY --from=build-env /opt/dcpam /opt/dcpam
WORKDIR /opt/dcpam/
EXPOSE 7771/tcp