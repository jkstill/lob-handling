OPTIONS (DIRECT=TRUE)
load data
infile 'blobsource.dat'
   "str '<EORD>'"
into table BLOBSOURCE
fields terminated by '<EOFD>'
(
	ID,
	B1 CHAR(2500000) 
)
