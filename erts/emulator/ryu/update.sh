#!/bin/bash
GITHUB_TOKEN=${GITHUB_TOKEN:-$(cat ~/.githubtoken)}
if [ -z "${GITHUB_TOKEN}" ]; then
    echo "You need to set ${GITHUB_TOKEN} to a valid github token"
    exit 1
fi
ORIGIN_REPO="git@github.com:erlang/ryu.git"
UPSTREAM_REPO="https://github.com/ulfjack/ryu"
cd $ERL_TOP/erts/emulator/ryu
if [ -d ryu-repo ]; then
    cd ryu-repo
    if ! git diff --exit-code HEAD; then
	cat <<EOF
There are uncommited changes in the ryu-repo repo.
Either commit the changes to continue
or remove the ryu-repo directory to start over.
Then run this script again.
EOF
	exit 1
    else
	cat <<EOF
There already is a ryu-repo directory.
EOF
	read -n1 -p "Do you want to continue using that repo? (y/n): " yn
	if [[ ! $yn =~ ^[Yy]$ ]]; then
	    cat <<EOF
Remove the repo ryu-repo to start over again.
EOF
	    exit 1
	fi
    fi
else
    git clone ${ORIGIN_REPO} ryu-repo
    cd ryu-repo
    git remote add upstream ${UPSTREAM_REPO}
    git fetch upstream
fi
set -exo pipefail
RYU_SHA=$(git rev-parse --verify upstream/master)
RYU_SHORT_SHA=$(git rev-parse --verify --short upstream/master)
if ! git merge upstream/master; then
    git status --short | grep "^D" | awk '{print $2}' | xargs git rm -rf
    git status --short | grep "^A" | awk '{print $2}' | xargs git rm -rf
    if ! git commit; then
	cat <<EOF
Merge of ${RYU_SHORT_SHA} to $(git rev-parse --short master) failed,
resolve and commit the conflict in $(pwd)
Then run this script again to continue.
EOF
	exit 1
    fi
fi
cd ..
STL_VSN=$(curl -sL -H "Authorization: Bearer ${GITHUB_TOKEN}" -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28"   https://api.github.com/repos/microsoft/STL/releases/latest | jq ".tag_name" | sed 's/"//g')
rm -rf STL
git clone --filter=blob:none --no-checkout -b ${STL_VSN} https://github.com/microsoft/STL.git STL
cd STL
    git sparse-checkout init --cone
    git sparse-checkout set stl/inc/xcharconv_ryu.h
    git checkout HEAD -- stl/inc/xcharconv_ryu.h
    STL_CHARCONV_SHA=$(git log --format="%H" HEAD -1  -- stl/inc/xcharconv_ryu.h)
    STL_SHA=$(git rev-parse HEAD)
cd ..
if [ "$(cat xcharconv_ryu.h.sha)" != "${STL_CHARCONV_SHA}" ]; then
    cat <<EOF
https://github.com/microsoft/STL/blob/master/stl/inc/xcharconv_ryu.h has been
updated. Check that no changes affect the __to_chars function and if they do
incorporate them into ryu-repo/to_chars.h.
Once done, update xcharconv_ryu.h.sha with the new sha. i.e.
echo "${STL_CHARCONV_SHA}" > ${ERL_TOP}/erts/emulator/ryu/xcharconv_ryu.h.sha
Then run this script again to continue.
EOF
    exit 1
fi
shopt -s extglob
git rm -rf $(ls -d !(update.sh|vendor.info|ryu.mk|obj|README.ryu_update.md|ryu-repo|STL|xcharconv_ryu.h.sha*))
shopt -u extglob
cp -r ryu-repo/* .
rm -rf STL
set -x
COMMENTS=$(cat vendor.info | grep "^//")
NEW_VENDOR_INFO=$(cat vendor.info | grep -v "^//" | jq "map(if .ID == \"erts-ryu\" then .versionInfo = \"${RYU_SHA}\" | .sha = \"${RYU_SHA}\" else . end)")
NEW_VENDOR_INFO=$(echo "${NEW_VENDOR_INFO}" | jq "map(if .ID == \"ryu-to_chars\" then .versionInfo = \"${STL_VSN}\" | .sha = \"${STL_SHA}\" else . end)")
cat <<EOF > vendor.info
${COMMENTS}
${NEW_VENDOR_INFO}
EOF
git add . ':!ryu-repo'
git commit -m "erts: Update ryu version to ${RYU_SHA}"
cat <<EOF
All commits done.
Verify it looks ok and then push result.
Don't forget to push to ${ORIGIN_REPO} as well:
      cd ryu-repo
      gitk
      git push origin master
      cd ..
      rm -rf ryu-repo
EOF