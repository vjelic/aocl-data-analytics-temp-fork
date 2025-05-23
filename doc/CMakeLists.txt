# ##############################################################################
# Copyright (c) 2023-2024 Advanced Micro Devices, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ##############################################################################
project(aoclda-doc)

find_package(Doxygen REQUIRED)
find_package(LATEX REQUIRED)
find_program(LATEXMK NAMES latexmk REQUIRED)
find_program(SED NAMES sed REQUIRED)
find_program(GREP NAMES grep REQUIRED)
find_program(SORT NAMES sort REQUIRED)
find_program(AWK NAMES awk gawk REQUIRED)

# Check for python and the documentation packages: - breathe -
# sphinxcontrib.bibtex
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "from rocm_docs import ROCmDocs"
  RESULT_VARIABLE EXIT_CODE
  OUTPUT_QUIET)
if(NOT ${EXIT_CODE} EQUAL 0)
  message(
    FATAL_ERROR
      "Python3 package rocm_docs is required to build the documentation (tested with version 0.34.2)\n   Use: ${Python3_EXECUTABLE} -m pip install \"rocm-docs-core<=0.34.2\""
  )
endif()
execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import breathe"
  RESULT_VARIABLE EXIT_CODE
  OUTPUT_QUIET)
if(NOT ${EXIT_CODE} EQUAL 0)
  message(
    FATAL_ERROR
      "Python3 package breathe is required to build the documentation\n   Use: ${Python3_EXECUTABLE} -m pip install breathe"
  )
endif()
execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import sphinxcontrib.bibtex"
  RESULT_VARIABLE EXIT_CODE
  OUTPUT_QUIET)
if(NOT ${EXIT_CODE} EQUAL 0)
  message(
    FATAL_ERROR
      "Python3 package sphinxcontrib.bibtex is required to build the documentation\n   Use: ${Python3_EXECUTABLE} -m pip install sphinxcontrib.bibtex"
  )
endif()
execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import sphinx_collapse"
  RESULT_VARIABLE EXIT_CODE
  OUTPUT_QUIET)
if(NOT ${EXIT_CODE} EQUAL 0)
  message(
    FATAL_ERROR
      "Python3 package sphinx_collapse is required to build the documentation\n   Use: ${Python3_EXECUTABLE} -m pip install sphinx_collapse"
  )
endif()

message(STATUS "Building AOCL-DA documentation version ${AOCLDA_VERSION}")

string(TIMESTAMP BUILD_DATE "%d %b %Y")

# This target makes sure all latex packages are available prior to launching the
# full build. build a here-document from the main doc which takes all required
# packages NOTE: this here-document needs to be kept in sync with refman.tex
add_custom_target(
  latexdeps.pdf
  COMMAND
    bash -c
    [[ echo -e '\\documentclass[twoside]{book}\n\\usepackage{ifthen}\n\\ifx\\requestedLaTeXdate\\undefined\n\\usepackage{array}\n\\else\n\\usepackage{array}[=2016-10-06]\n\\fi\n\\usepackage{fixltx2e}\n%\\usepackage{doxygen}\n\\usepackage{graphicx}\n\\usepackage[utf8]{inputenc}\n\\usepackage{makeidx}\n\\PassOptionsToPackage{warn}{textcomp}\n\\usepackage{textcomp}\n\\usepackage[nointegrals]{wasysym}\n\\usepackage{ifxetex}\n\\usepackage[T1]{fontenc}\n\\usepackage[scaled=.90]{helvet}\n\\usepackage{courier}\n\\usepackage{sectsty}\n\\usepackage{geometry}\n\\usepackage{fancyhdr}\n\\usepackage{natbib}\n\\usepackage[titles]{tocloft}\n\\usepackage{amsmath}\n\\usepackage{newunicodechar}\n\\ifpdf\n\\usepackage[pdftex,pagebackref=true]{hyperref}\n\\else\n\\ifxetex\n\\usepackage[pagebackref=true]{hyperref}\n\\else\n\\usepackage[ps2pdf,pagebackref=true]{hyperref}\n\\fi\n\\fi\n\\usepackage{etoc}\n\\begin{document}\n\\end{document}' > latexdeps.tex ]]
  COMMAND
    bash -c
    [[ pdflatex -halt-on-error -interaction nonstopmode latexdeps.tex ; if [ $? -ne 0 ] ; then echo -e '\n! LaTeX Error: Could not check for the required dependencies. Check latexdeps.log for\nmore details and make sure your LaTeX distribution has the following packages:' ; grep usepackage latexdeps.tex | sed 's/^ *\\usepackage.*{\(.*\)}.*$/\1/' | sort -u  ; echo ; exit 1 ; fi ]]
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Checking for LaTeX dependencies..."
  VERBATIM
  SOURCES Doxyfile)

add_custom_target(
  clean-doc
  COMMAND cd ${PROJECT_BINARY_DIR} && rm -vrf html/ latex/ rtf/ xml/ sphinx/
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Cleaning temporary documentation files"
  VERBATIM)

add_dependencies(cleanall clean-doc)

# Build sphinx Documentation. DOC_TYPE controls what document to produce and can
# be set to either pdf or html, default is html
#
# Inspired from Sphinx in CMake section of MSFT blog post
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Add Sphinx doc target
find_package(Sphinx REQUIRED)

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(PDF_DOC_NAME AOCL-DA_Documentation_${AOCLDA_VERSION})
option(INTERNAL_DOC "Build internal documentation" ON)
if(INTERNAL_DOC)
  set(SPHINX_TAG "-t" "internal")
else()
  set(SPHINX_TAG "")
endif()


# Target to regenerate ALL registered options in handle (datastore)
# These are tables stored in a file and added as a chapter (appendix?)
# in the doc.
# It requires the unit test to be built, otherwise it won't build
# the target.
# | ${GREP} -v -e ^Running -e \\\[.\\\+\\\]
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/process.awk
     CONTENT
     "BEGIN {it=0}
     /^Running/ { next }
     /\\[=+\\]/ { exit }
     { if (it==1) { printf \"%3s\" , \"\" } print $0 }
     /^\\.\\.\\sonly:: internal/ { it=1 }")

add_custom_target(
    gen_options_tables
    COMMAND doc_internal ARGS --gtest_filter=DocOptions*.RST*
      --gtest_print_time=0 --gtest_brief=1 --gtest_color=no
    | ${AWK} -f ${CMAKE_CURRENT_BINARY_DIR}/process.awk
    > ${SPHINX_SOURCE}/options/all_table.rst
    COMMAND ${CMAKE_SOURCE_DIR}/tools/insert_license_info.py ${SPHINX_SOURCE}/options/all_table.rst
    DEPENDS doc_internal
    COMMENT "Running unit-test for doc options tables"
    VERBATIM
)

# Search all the ReSt files for option tables and update them with the
# latest database (gen_options_tables)
file(GLOB_RECURSE REST_FILES
  "${PROJECT_SOURCE_DIR}/*.rst"
)
## A note of caution: this target breaks when jobs>1
add_custom_target(
    update_options_tables
    WORKING_DIRECTORY ${AOCL-DA_SOURCE_DIR}
    COMMAND tools/update_options_tables.sh
    --db=${PROJECT_SOURCE_DIR}/options/all_table.rst ${REST_FILES}
    COMMENT "Search and update ReSt files doc/*.rst"
)


# Target building the doxygen XML. Also modifies the output directory and the
# version number in the Doxyfile: 1. replace PROJECT_NAME = .* for the string
# "version ${AOCLDA_VERSION} (date)" 2. Run doxygen on the sources
add_custom_target(
  doxygen_xml
  # Version bump and set the correct output directory
  # "s/^\\(PROJECT_NUMBER[[:space:]]*=\\).*$/\\1 \"version ${AOCLDA_VERSION} (${BUILD_DATE})\"/"
  COMMAND
    ${SED} -i -e
    "s/^\\(PROJECT_NUMBER[[:space:]]*=\\).*$/\\1 \"Release ${AOCLDA_VERSION}\"/"
    -e "s+^\\(OUTPUT_DIRECTORY[[:space:]]*=\\).*$+\\1 ${PROJECT_BINARY_DIR}+"
    Doxyfile
    # Run doxygen THEN reset the OUTPUT_DIRECTORY
  COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile &&
    ${SED} -i -e
    "s+^\\(OUTPUT_DIRECTORY[[:space:]]*=\\).*$+\\1 auto-generated-by-cmake+"
    Doxyfile
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  VERBATIM
  SOURCES Doxyfile
  DEPENDS update_options_tables
  COMMENT "Building doxygen xml files")

# Bump version string in conf.py (Sphinx)
add_custom_target(
  conf_py_version_bumping
  COMMAND
    ${SED} -i -e
    "s/^\\(release[[:space:]]*=\\).*$/\\1 \'${AOCLDA_VERSION}\'/"
    conf.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  VERBATIM
  COMMENT "[sphinx] conf.py: version ${AOCLDA_VERSION}")

# Add table of options if available
if (${BUILD_GTEST})
    message(STATUS "Documentation: new options tables will be generated.")
    add_dependencies(doxygen_xml gen_options_tables)
endif()

# Build the html documentation in doc/sphinx/html
add_custom_target(
  doc_html
  COMMAND
    ${SPHINX_EXECUTABLE} ${SPHINX_TAG} -b html
    -Dbreathe_projects.aocl-da=${DOXYGEN_OUTPUT_DIR}/xml ${SPHINX_SOURCE}
    ${SPHINX_BUILD}/html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS doxygen_xml conf_py_version_bumping
  COMMENT "Generating HTML documentation")

# Build latex documentation in doc/sphinx/latex
add_custom_target(
  doc_pdf
  COMMAND
    ${SPHINX_EXECUTABLE} ${SPHINX_TAG} -b latex
    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.aocl-da=${DOXYGEN_OUTPUT_DIR}/xml ${SPHINX_SOURCE}
    ${SPHINX_BUILD}/latex/
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
    cd ${SPHINX_BUILD}/latex && make && mv ${SPHINX_BUILD}/latex/aocl-da.pdf
    ${SPHINX_BUILD}/latex/${PDF_DOC_NAME}.pdf
  DEPENDS latexdeps.pdf doxygen_xml conf_py_version_bumping
  COMMENT "Generating PDF documentation")

# Main target doc, builds both html and pdf
add_custom_target(doc DEPENDS doc_pdf doc_html)

# Courtesy "Release Notes" section in the doc
write_file(${PROJECT_BINARY_DIR}/ReleaseNotes.md "# Release Notes\n\n")
write_file(${PROJECT_BINARY_DIR}/ReleaseNotes.md
           "AOCL-DA version: ${AOCLDA_VERSION}\n" APPEND)
write_file(${PROJECT_BINARY_DIR}/ReleaseNotes.md
           "OS: ${CMAKE_CXX_PLATFORM_ID}\n" APPEND)
write_file(
  ${PROJECT_BINARY_DIR}/ReleaseNotes.md
  "compiler: ${CMAKE_CXX_COMPILER}: ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})\n"
  APPEND)
write_file(
  ${PROJECT_BINARY_DIR}/ReleaseNotes.md
  "compiler: ${CMAKE_Fortran_COMPILER}: ${CMAKE_Fortran_COMPILER_ID} (${CMAKE_Fortran_COMPILER_VERSION})\n"
  APPEND)
write_file(${PROJECT_BINARY_DIR}/ReleaseNotes.md
           "Flags: <see ${CMAKE_BINARY_DIR}/compile_commands.json>\n" APPEND)
