#!/bin/bash
daisysp_dir=./

# create folder
mkdir -p doc

# Generate doxygen HTML
echo "Creating doxygen documentation"
doxygen doc/Doxyfile > /dev/null

# Generate reference manual PDF
reference_manual_pdf_name=doc/libdaisy_reference.pdf
doxygen_latex_dir=doc/latex
doxygen_latex_outfile=doc/latex/refman.pdf

if [ "$DEPLOY_DOCS" = "1" ]; then
    echo "Installing additional packages for latex PDF generation"
    sudo apt-get install texlive-fonts-recommended texlive-latex-extra texlive-fonts-extra dvipng texlive-latex-recommended lmodern 

    if ! [ -x "$(command -v pdflatex)" ]; then
        echo 'Warning: no reference manual will be created. Please install pdflatex.' >&2
    else
        echo "Generating reference manual PDF"
        # remember current directory
        cwd=$(pwd)
        cd $doxygen_latex_dir
        make pdf > /dev/null
        # go back and move the complete pdf file
        cd $cwd
        mv $doxygen_latex_outfile $reference_manual_pdf_name
    fi
else 
    echo 'Warning: Skipping PDF generation - to generate PDF documents, set DEPLOY_DOCS = 1'
fi

# Generate Style Guide
# echo "Creating Style Guide PDF"
# pdf_name=doc/style_guide.pdf
# pandoc ./resources/Style-Guide.md  -s --template=./resources/template.tex --toc -o $pdf_name

echo "finished."
