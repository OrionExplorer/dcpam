FROM ubuntu:18.04 as build-env

RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "make"\
 "clang"\
 && apt-get clean\
 && rm -rf /var/lib/apt/lists/*

COPY conf/lcs_config.json /dcpam/conf/
COPY src/ /dcpam/src/
COPY makefile.lcs /dcpam/
WORKDIR /dcpam/
RUN make -f makefile.lcs
RUN rm ./*.o && rm makefile.lcs && rm ./src -rf

FROM ubuntu:18.04
COPY --from=build-env /dcpam /dcpam
WORKDIR /dcpam/
EXPOSE 7771/tcp