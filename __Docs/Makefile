#
# Build the documentation from the DocBook XML
#

html:
	rm -rf _html
	mkdir _html
	cp docbook.css _html
	cd _html; xmlto --with-dblatex xhtml -m ../config_html.xsl ../ReadMe.xml

pdf:
	xmlto --with-dblatex pdf ReadMe.xml
