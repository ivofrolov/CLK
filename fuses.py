Import('env')
env.Replace(FUSESCMD="$UPLOADER $UPLOADERFLAGS -e -U lfuse:w:0xf7:m -U hfuse:w:0xde:m -U efuse:w:0xfd:m")
