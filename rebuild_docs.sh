#!/bin/bash
daisysp_dir=daisysp
libdaisy_dir=libdaisy

# Generate MD

## daisysp
echo "creating daisysp markdown files. . . "
for header in $daisysp_dir/modules/*.h; do
    fname=${header##*/}
    fname=${fname%.h}
    outfile=$daisysp_dir/doc/$fname.md
    echo "    "$fname
    python makedoc.py $header > $outfile
done
echo "done."

## libdaisy 
## TODO: handle multiple directories here...
echo "creating libdaisy markdown files. . . "
for header in $libdaisy_dir/src/*.h; do
    fname=${header##*/}
    fname=${fname%.h}
    outfile=$libdaisy_dir/doc/$fname.md
    echo "    "$fname
    python makedoc.py $header > $outfile
done

echo "done."

# Generate PDFs

## daisysp
pdf_name=daisysp
cat resources/preamble.md > temp.md
echo "\pagebreak" >> temp.md
for file in $daisysp_dir/doc/*.md; do
    cat $file >> temp.md
    echo "\pagebreak" >> temp.md
done
echo "Generating documenation for " $pdf_name
pandoc temp.md -s --template=./resources/template.tex --pdf-engine=xelatex --toc -o $pdf_name.pdf
echo "Created $pdf_name.pdf"
# remove temp file.
rm temp.md
echo "done."
pdf_name=libdaisy
cat resources/preamble.md > temp.md
echo "\pagebreak" >> temp.md
for file in $libdaisy_dir/doc/*.md; do
    cat $file >> temp.md
    echo "\pagebreak" >> temp.md
done
echo "Generating documenation for " $pdf_name
pandoc temp.md -s --template=./resources/template.tex --pdf-engine=xelatex --toc -o $pdf_name.pdf
echo "Created $pdf_name.pdf"
# remove temp file.
rm temp.md
echo "done."
echo "finished."

