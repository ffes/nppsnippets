#!/usr/bin/env bash

PATH=/usr/bin:/bin

# Make sure there is a file, but don't touch it if not needed
if ! [ -f version_git.h ]
then
	touch version_git.h
fi

# Is git installed?
if [ -x /usr/bin/git ]
then
	# Get the latest git commit
	VERSION=$(git describe --tags)

	# Has the version changed?
	if ! grep --quiet $VERSION version_git.h
	then
		echo "Generating version_git.h..."
		echo "#pragma once" > version_git.h
		echo "#define VERSION_GIT_STR \"$VERSION\"" >> version_git.h
		echo "#define VERSION_GIT_WSTR L\"$VERSION\"" >> version_git.h
	fi
else
	echo "Git not found, generating dummy version_git.h..."
	echo "#pragma once" > version_git.h
	grep VERSION_NUMBER_ Version.h | sed 's/NUMBER/GIT/' >> version_git.h
fi
