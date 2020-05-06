#!/usr/bin/env bash

function check_if_installed()
{
	# Go through all the tools that need to be checked
	for i in $*
	do
		# Is tool installed?
		if ! type "$i" >/dev/null 2>&1
		then
			echo "Required tool \"$i\" not found!"
			exit 1
		fi
	done
}

function create_version_git_h()
{
	# Make sure there is a file, but don't touch it if not needed
	[[ -f version_git.h ]] || touch version_git.h

	# Make sure the locale is set properly
	export LANG=en_US.UTF8

	# Get the version info from git
	echo "Retrieving version information from git..."
	GIT_VERSION=$(git describe --tags --match 'v[0-9]*')
	echo "Found version $GIT_VERSION"

	# Has the version changed?
	if grep --quiet $GIT_VERSION version_git.h
	then
		echo "Latest version info already in version_git.h"
		exit 0
	fi

	# Get additional version info from git
	echo "Retrieving additional version information from git..."
	VERSION=$(git describe --tags --abbrev=0 | grep -oP '\d+\.\d+\.\d+')
	VERSION_NUMBERS=$(echo $VERSION | tr "." ","),0
	YEAR=$(date +%Y)

	echo "Generating version_git.h..."
	echo "#pragma once" > version_git.h

	echo "#define VERSION_NUMBER $VERSION_NUMBERS" >> version_git.h
	echo "#define VERSION_NUMBER_STR \"$VERSION\"" >> version_git.h
	echo "#define VERSION_NUMBER_WSTR L\"$VERSION\"" >> version_git.h
	echo "#define COPYRIGHT_STR \"Copyright (c) 2010-$YEAR by Frank Fesevur\"" >> version_git.h

	echo "#define VERSION_GIT_STR \"$GIT_VERSION\"" >> version_git.h
	echo "#define VERSION_GIT_WSTR L\"$GIT_VERSION\"" >> version_git.h
}

# Check if all the tools we need are installed
check_if_installed git grep tr date

create_version_git_h
