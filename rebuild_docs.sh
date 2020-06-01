#!/bin/bash
libdaisy_dir=./

# Generate MD

#TODO:
# * Add ignored prefix functionality for the following file prefixes:
#     * usbd_
#     * system_ (note: not sys_)

## libdaisy
echo "creating libdaisy markdown files. . . "
for header in $libdaisy_dir/src/*.h; do
    fname=${header##*/}
    fname=${fname%.h}
    ex=$libdaisy_dir/examples/$fname/ex_$fname.cpp
    outfile=$libdaisy_dir/doc/$fname.md
    echo "    "$fname
    python makedoc.py $header > $outfile
    # add example to the end of each doc.
    #echo $'\n## Example\n' >> $outfile
    #if [ -f "$ex" ]; then
    #    echo $'```C++\n' >> $outfile
    #    cat $ex >> $outfile
    #    echo $'\n```' >> $outfile
    #else
    #    echo "No example Provided for $fname"
    #    echo "No example Provided" >> $outfile
    #fi
done
echo "done."


# Generate PDFs

mkdir -p doc
## libdaisy
pdf_name=doc/libdaisy_reference.pdf
cat resources/preamble.md > temp.md
echo "\pagebreak" >> temp.md
for file in $libdaisy_dir/doc/*.md; do
    cat $file >> temp.md
    echo "\pagebreak" >> temp.md
done
echo "Generating documenation for " $pdf_name
#pandoc temp.md -s --template=./resources/template.tex --toc -o $pdf_name
pandoc temp.md -s --pdf-engine=xelatex --template=./resources/template.tex --toc -o $pdf_name
echo "Created $pdf_name"
# remove temp file.
rm temp.md
echo "done."
echo "finished."


