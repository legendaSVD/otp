#!/bin/bash
GITHUB_TOKEN=${GITHUB_TOKEN:-$(cat ~/.githubtoken)}
if [ -z "${GITHUB_TOKEN}"]; then
    echo "You need to set ${GITHUB_TOKEN} to a valid github token"
    exit 1
fi
cd $ERL_TOP/erts/emulator/zstd
set -eo pipefail
shopt -s extglob
git rm -rf $(ls -d !(update.sh|vendor.info|zstd.mk|obj))
shopt -u extglob
VSN=$(curl -sL -H "Authorization: Bearer ${GITHUB_TOKEN}" -H "Accept: application/vnd.github+json"   -H "X-GitHub-Api-Version: 2022-11-28"   https://api.github.com/repos/facebook/zstd/releases/latest | jq ".tag_name" | sed 's/"//g')
git clone https://github.com/facebook/zstd -b $VSN zstd-copy
SHA=$(cd zstd-copy && git rev-parse --verify HEAD)
cp -r zstd-copy/lib/{common,compress,decompress} ./
cp zstd-copy/LICENSE zstd-copy/COPYING ./
API_HEADERS="zstd.h zdict.h zstd_errors.h"
for API_HEADER in ${API_HEADERS}; do
    cp "zstd-copy/lib/${API_HEADER}" "erl_${API_HEADER}"
    sed -i "s@../${API_HEADER}@../erl_${API_HEADER}@g" ./*/*.{c,S,h}
    sed -i "s@${API_HEADER}@erl_${API_HEADER}@g" ./*.h
done
rm -rf zstd-copy
COMMENTS=$(cat vendor.info | grep "^//")
NEW_VENDOR_INFO=$(cat vendor.info | grep -v "^//" | jq "map(if .ID == \"erts-zstd\" then .versionInfo = \"${VSN}\" | .sha = \"${SHA}\" else . end)")
cat <<EOF > vendor.info
${COMMENTS}
${NEW_VENDOR_INFO}
EOF
git add common compress decompress ./erl_*.h COPYING LICENSE vendor.info
git commit -m "erts: Update zstd version to ${VSN}"