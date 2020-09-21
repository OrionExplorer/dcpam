FROM ubuntu:18.04 as build-env

RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "clang"\
 "libaio-dev"\
 "libpq-dev"\
 "libmariadbclient-dev"\
 "unixodbc-dev"\
 "alien"\
 "wget"\
 && apt-get clean\
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
RUN wget https://download.oracle.com/otn_software/linux/instantclient/19600/oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm --no-check-certificate
RUN alien -i oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm
RUN wget https://download.oracle.com/otn_software/linux/instantclient/19600/oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm --no-check-certificate
RUN alien -i oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm
ENV LD_LIBRARY_PATH=/usr/lib/oracle/19.6/client64/lib/${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
ENV PATH=$PATH:$ORACLE_HOME/bin
RUN ln -s /usr/include/oracle/19.6/client64/ $ORACLE_HOME/include

COPY conf/etl_config.json /dcpam/conf/
COPY src/ /dcpam/src/
COPY makefile.etl /dcpam/
WORKDIR /dcpam/
RUN make -f makefile.etl
RUN rm ./*.o && rm makefile.etl && rm ./src -rf

FROM ubuntu:18.04
RUN apt-get update\
 && apt-get install -y --no-install-recommends\
 "libaio-dev"\
 "libpq-dev"\
 "libmariadbclient-dev"\
 "unixodbc-dev"\
 "alien"\
 "wget"\
 && apt-get clean\
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
COPY --from=build-env /tmp/oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm /tmp/
RUN alien -i oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm
RUN rm ./oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm
COPY --from=build-env /tmp/oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm /tmp/
RUN rm ./oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm
ENV LD_LIBRARY_PATH=/usr/lib/oracle/19.6/client64/lib/${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
ENV PATH=$PATH:$ORACLE_HOME/bin
RUN ln -s /usr/include/oracle/19.6/client64/ $ORACLE_HOME/include

COPY --from=build-env /dcpam /dcpam
WORKDIR /dcpam
EXPOSE 8888/tcp