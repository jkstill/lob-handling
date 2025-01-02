
blob.c is being developed on lestrade

currently it compiles without error, and updates the b1 blob column

the good news is that is is very fast

It is now working correctly as a prototype

clob-to-blob.c

Using optimizer hex to binary function found on StackOverflow

This function uses a look up table to convert hex to binary - it is very fast.


## 19c, 21c and 23ai versions

`make-clob-to-blob.sh` compiled cleanly with all versions.

While all are the same size, they are not necessarily the same.

```text
$  md5sum clob-to-blob-ocilib-static-*
dea942397c5d135a16b9c7d4bc5894f5  clob-to-blob-ocilib-static-19c
dea942397c5d135a16b9c7d4bc5894f5  clob-to-blob-ocilib-static-21c
ec145dc50b5cd0575c4d63b8bb6ad7d0  clob-to-blob-ocilib-static-23ai
```




