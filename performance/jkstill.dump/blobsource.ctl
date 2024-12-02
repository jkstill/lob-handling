OPTIONS (DIRECT=TRUE)
load data
infile 'blobsource.dat'
   "str '<EORD>'"
into table BLOBSOURCE
fields terminated by '<EOFD>'
(
	ID,
	NAME,
	B1 CHAR(5000000) 
)
