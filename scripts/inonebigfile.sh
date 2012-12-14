

for FILE in `ls *.log`
do
    echo "File: $FILE "
    # dateiname in tabs trennen
    NAME=`basename $FILE .log | tr '-' '\t'`

    # geht nicht weil die zeile nochmal unterbrochen wird
    IFS="\n"
    for LINE in $(cat $FILE)
    do
        echo -e "$NAME\t$LINE" >>./onebigfile
    #    echo -ne "$NAME\t" >>$OBF
    #    cat $i >>$OBF
    done

done

exit