FROM sphinxdoc/sphinx

WORKDIR /docs

# Install the required packages for Python Sphinx
ADD requirements.txt /docs
RUN pip3 install -r requirements.txt

CMD ["make", "html"]
