import codecs
from setuptools import setup, find_packages

# with codecs.open('README.md', 'r', 'utf8') as reader:
#     long_description = reader.read()

setup(name='new_word_extraction',
      version='0.0.1g',
      description='new-word-extraction',
      long_description="",
      author='Pan Haojie',
      author_email='jasonhjpan@tencent.com',
      install_requires=[
            'six>=1.9.0',
            'jieba'],
      extras_require={
          'tests': ['pytest'],
      },
      classifiers=(
          "Programming Language :: Python :: 3.6",
          "Operating System :: OS Independent",
      ),
      package_data={
          '': ['data/*.*'],
      },
      packages=find_packages(),
      )
