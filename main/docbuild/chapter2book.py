#!/usr/bin/env python

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

"""
	@file chapter2book.py

	@author Jeromy Tompkins

	This script promotes a chapter from the big documentation book into a complete
	docbook book. The user provides the location of the chapter source (this
	should only contain the user guide type information) as well as the location
	of the book info (i.e. title, author, revisions, etc). If the book info is
	missing, the product of this script will simply lack an author and the title
	will be the same as the chapter title. However, if the book info is provided,
	the current chapter title is discarded and the title, author, and revision
	history are taken from the info. The whole scheme works because the chapter
	and section tags in docbook take similar child tags. For this to work though,
	you should avoid putting bare paragraph tags in the chapter. The best results
	are found when the document is formatted as..

  @verbatim
	<chapter>
	  <title>Title</title>
		
		<section>
			<title>asdaf</title>
			<para>...</para>

		</section>

		<!-- more sections -->
	</chapter>
	@endverbatim

	In the absense of book info, this will become:
  
	@verbatim
	<book>
		<title>Title</title>

		<chapter>
			<title>asdaf</title>
			<para>...</para>
		</chapter>

		<!-- more chapters -->
	</book>
  @endverbatim

  When the book info is provided, this will become:

	@verbatim
	<book>
		<bookinfo>
		<title>Title</title>
		<author>....</author>
		<!-- other stuff -->
		</bookinfo>

		<chapter>
			<title>asdaf</title>
			<para>...</para>
		</chapter>

		<!-- more chapters -->
	</book>
  @endverbatim

	Be wary that this script is not terribly smart. It does not check to see that
	the input xml tree will convert to proper docbook. It just converts the
	toplevel tags, inserts at most a single child node, and then saves the
	resulting xml tree to a new file. If desired, docbook2pdf is run on the new
	file. The real test for whether the input xml was valid is determined by
	whether docbook2pdf complains that the new xml tree is invalid docbook. 

"""

# import the necessary packages
import sys
import os
from os import path
try:
	import xml.etree.cElementTree as etree
except ImportError:
	import xml.etree.ElementTree as etree
import argparse
import subprocess


#############################################################################
# Set Up the argument parser
parser = argparse.ArgumentParser()
parser.add_argument('-s','--source',
										help='Path to the xml defining body', 
										required=True)
parser.add_argument('-b','--bookinfo',
										help='Path to the xml file contain bookinfo element')
parser.add_argument('-x','--xml',
										help='Save the xml generated for the book',
										type=bool,
										default=False,
										nargs='?')
parser.add_argument('-p','--pdf',
										help='Generate the pdf',
										type=bool,
										default=True,
										nargs='?')
parser.add_argument('-o','--outdir', help='Directory of output files',
										default='.')

#############################################################################

class ChapterUpgrader:
	"""
	Reads in an xml tree from file and upgrades all
	first level section tags to chapter tags. 
	"""
	tree = None
	def __init__(self,path):
		"""
		@brief Constructor

	  Reads in the tree from a path and then immediately upgrades the sections
		It also changes the root tag from chapter to book.

		@param path		the path to the xml content tree 

		"""
		self.tree = etree.parse(path)
		root = self.tree.getroot()
		root.tag = 'book'
		self.sections_to_chapters(root)
	
	def sections_to_chapters(self,root):
		"""
		@brief Upgrades the section tags to chapters

		@param root		the root element (type=etree.Element) 

		"""
		for element in root:
			if element.tag == 'section':
				element.tag = 'chapter'
	
	def get_tree(self):
		return self.tree
	
	def insert_bookinfo(self,path):
		"""
		@brief Inserts the bookinfo tag

		Deletes the first top-level title tag and replaces it with the bookinfo
		element that is read from the provided file

		@param path		path to the xml file containing <bookinfo> tag

		"""
		infoRoot = etree.parse(path).getroot()
		root = self.tree.getroot()
		
		title = root.find('title')
		if title != None:
			root.remove(title)
		
		root.insert(0,infoRoot)

#---- End of ChapterUpgrader class  

# BEGIN SCRIPT

args = parser.parse_args()

# Set up some useful names
targetFile     = args.source
baseTargetFile = path.splitext(os.path.basename(targetFile))[0] # filename without path or extension
upgradedFile   = path.join('.',baseTargetFile+'_upgr.xml')
outputFile   = path.join(args.outdir,baseTargetFile+'_autobook.xml')
bookInfoFile   = args.bookinfo

# upgrade chapter to book and insert bookinfo if provided
upgradedTree = ChapterUpgrader(targetFile)
if bookInfoFile:
	upgradedTree.insert_bookinfo(bookInfoFile)

# convert new xml tree to a string
textTree = etree.tostring(upgradedTree.get_tree().getroot())

# insert the declaration
docbook_decl = '<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN" ' \
          			' "file:///usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">'


# write the new xml tree to an output file
newfile = open(outputFile,'w+')
newfile.write('<?xml version="1.0" encoding="ASCII"?>\n')
newfile.write(docbook_decl+'\n')
newfile.write(textTree.decode('utf-8'))
newfile.write('\n')
newfile.close()

# generate the pdf is desired
# Note this is a bit nasty and not quite right because
# really autoconf should be determining how to build pdf's from .xml's.
#
if args.pdf:
	currentWd = os.getcwd()
	print(currentWd)
	subprocess.call(['docbook2pdf','-o',args.outdir,outputFile])

# clean up generated xml if requested
if not args.xml:
	os.remove(outputFile)
