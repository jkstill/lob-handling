load data
infile 'blobtest.csv'
into table BLOBTEST
fields terminated by ',' optionally enclosed by '"'
(
	ID,
	C1,
	B1 "hex_to_raw(:B1)"
)
