FROM scratch as unpack
ADD Circuit-0.1.1-Linux.tar.gz /
FROM ubuntu:24.04
COPY --from=unpack --link /Circuit-0.1.1-Linux/bin/FindAllOutputs /usr/local/bin/find-all-outputs