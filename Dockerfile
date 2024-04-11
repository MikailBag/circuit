FROM scratch as unpack-debug
ADD debug.tar.gz /
FROM scratch as unpack-release
ADD release.tar.gz /
FROM scratch as unpack-sanitize
ADD sanitize.tar.gz /
FROM ubuntu:24.04
COPY --from=unpack-debug --link /debug/bin/FindAllOutputs /usr/local/bin/find-all-outputs-debug
COPY --from=unpack-release --link /release/bin/FindAllOutputs /usr/local/bin/find-all-outputs-release
COPY --from=unpack-sanitize --link /sanitize/bin/FindAllOutputs /usr/local/bin/find-all-outputs-sanitize