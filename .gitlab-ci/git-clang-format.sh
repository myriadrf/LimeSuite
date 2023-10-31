#!/bin/sh
# Based on Github Action
# https://github.com/yshui/git-clang-format-lint

COMMIT="$CI_MERGE_REQUEST_DIFF_BASE_SHA"

if [ "$COMMIT" = "" ]; then
  COMMIT=$CI_COMMIT_BEFORE_SHA
fi

diff=$(git-clang-format --diff --commit "$COMMIT")
[ "$diff" = "no modified files to format" ] && exit 0
[ "$diff" = "clang-format did not modify any files" ] && exit 0

printf "\033[1mYou have introduced coding style breakages, suggested changes:\n\n"

echo "$diff" | colordiff
exit 1
