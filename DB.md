# Process EHR DB 

---

## Extract age specific patients 

```
Program information:
  -h [ --help ]          print help message.
  -V [ --version ]       print version number

Usage:
  -D [ --dbfile ] arg    database file []
  -A [ --agemax ] arg    Max Age [5]
  -a [ --agemin ] arg    Min Age [0]
  -o [ --obfile ] arg    output file [out.dat]



```


### Usage:

```
./bin/extract_age_group -D /run/media/ishanu/D3/DXRXPX/tPATT/data/CASES_DX.txt -A 20 -o data/DX_age20.dat./bin/extract_age_group -D /run/media/ishanu/D3/DXRXPX/tPATT/data/CASES_DX.txt -A 20 -o data/DX_age20.dat

```
