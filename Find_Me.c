/*
 * IP GeoLocation Application, C language, 2020
 * Abu Muid Md. Raafee
 * 
 * GOAL: Show location of any internet connected computer running their code.
 * 
 * DESCRIPTION: Returns the lattitude and longitude of the system's current 
 * location based on the system's external IP address. It also returns the 
 * accuracy of the retrieved location as a radius measured in Kilometers.
 */

#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h> // Ensure proper path to the curl header files

#define BUFSIZE 256

/* Standard data-chunk-reading structure used with libcurl */
struct web_data {
	char *buffer;
	size_t size;
};

/* Structure to store information about web pages */
struct location {
	char address[BUFSIZE];
	double latitude;
	double longitude;
};

void parse(char *json, char *key, char *val);
void extractValue(char *json, char *key, char *val);
void fetch(int v, char *o,char *f);
static size_t write_mem(void *ptr, size_t size, size_t nmemb, void *userdata);


int main()
{
	CURL *curl;
	CURLcode res;
	char csv_field[BUFSIZE],ip[BUFSIZE],value[BUFSIZE];
	char *offset;
	int x;
	double avg_lon,avg_lat,tot_lon,tot_lat,max_lon,max_lat,rad_lon,rad_lat,radius;
	struct location url[3];
	struct web_data curl_data[3];

	/* Initialize Structures */
	/* curl_data and url structures must be kept separate or the
	   call the curl makes to write_mem() screws up */
	for(x=0;x<3;x++)
	{
		curl_data[x].buffer = malloc(1);
		curl_data[x].size   = 0;
		url[x].latitude     = 0.0;
		url[x].longitude    = 0.0;
	}
	/* Initialize URL addresses */
	strcpy(url[0].address,"http://ip-api.com/csv/");
	strcpy(url[1].address,"https://tools.keycdn.com/geo.json?host=");
	strcpy(url[2].address,"https://ipinfo.io/");

	/* Initialuze curl */
	curl = curl_easy_init();


	/*---------------- 1ST READ ----------------*/

	/* Setting Options */
		// Specifying URL to read
	curl_easy_setopt(curl, CURLOPT_URL, url[0].address);
		// Specifying function for reading in data chunks
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem);
		// Specifying structure to use for reading
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_data[0]);
	
	/* Perform the call */
	res = curl_easy_perform(curl);
	/* Confirm if call was successful, bail if not */
	if( res != CURLE_OK )
	{
		fprintf(stderr,"1st Curl read failed: %s\n", curl_easy_strerror(res));
		exit(1);
	}

	/* At this point, the size of the data read is stored in curl_data.size
	   and the string read is in curl_data.buffer. The data is in CSV format,
	   which the fetch() function can read */
	/* Was the call successful? Fetch the first CSV item from the buffer and
	   store it in buffer 'csv_field' */
	fetch(1,curl_data[0].buffer,csv_field);
	/* The call failed if the string csv_field doesn't equals to 'success' */
	if( strncmp(csv_field,"success",7) != 0 )
	{
		fprintf(stderr,"Failed request from server: %s\n",url[0].address);
		fprintf(stderr,"Retried status: %s\n",csv_field);
		exit(1);
	}

	/* Retrieve the latitude value & convert it to double */
	fetch(8,curl_data[0].buffer,csv_field);
	url[0].latitude = strtod(csv_field,NULL);
	/* Retrieve the longitude value & convert it to double */
	fetch(9,curl_data[0].buffer,csv_field);
	url[0].longitude = strtod(csv_field,NULL);
	/* To use in the next call fetch this system's Internet IP */
	/* Also remove the unwanted newline at the end of ip string */
	fetch(14,curl_data[0].buffer,ip);
	ip[strlen(ip)-1] = '\0';


	/*---------------- 2ND READ ----------------*/

	/* Reset curl and Start over */
	curl_easy_reset(curl);
	/* Append the IP address to the nxt URL */
	strcat(url[1].address,ip);

	/* Configuring the call */
	curl_easy_setopt(curl, CURLOPT_URL, url[1].address);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_data[1]);
	/* Read the page */
	res = curl_easy_perform(curl);
	if( res != CURLE_OK )
	{
		fprintf(stderr,"2nd curl read failed: %s\n", curl_easy_strerror(res));
		exit(1);
	}

	/* Now, the buffer contains JSON data for the given IP */
	/* We need to confirm that the call was successful */
	parse(curl_data[1].buffer,"status",value);
	if( strcmp(value,"success")==0 )
	{
		/* Extract the latitude key */
		extractValue(curl_data[1].buffer,"latitude",value);
		url[1].latitude = strtod(value,NULL);
		/* Extract the longitude key */
		extractValue(curl_data[1].buffer,"longitude",value);
		url[1].longitude = strtod(value,NULL);
	}
	else
	{
		printf("Read from site %s unsuccessful\n",url[1].address);
	}
	

	/*---------------- 3RD READ ----------------*/
	
	/* Reset curl and Start over */
	curl_easy_reset(curl);
	/* Append the IP to the address */
	strcat(url[2].address,ip);
	/* Also append the json string to the address */
	strcat(url[2].address,"/json");

	/* Configure the call */
	curl_easy_setopt(curl, CURLOPT_URL, url[2].address);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_data[2]);
	/* Read the page */
	res = curl_easy_perform(curl);
	if( res != CURLE_OK )
	{
		fprintf(stderr,"3rd curl read failed: %s\n",
				curl_easy_strerror(res)
			   );
		exit(1);
	}

	/* Page buffer contains json data. Look for the key "loc" */
	parse(curl_data[2].buffer,"loc",value);
	/* Key "loc" has two values, separated by a comma as in "0.0,0.0" */
		// Find the comma
	offset = value;
	while(*offset!=',')
	{
		if(*offset=='\0')
		{
			fprintf(stderr,"Unable to parse 'loc' data\n");
			exit(1);
		}
		offset++;
	}
	/* Cap the 1st string (latitude) and point offset at the 2nd, longitude */
	*offset='\0';
	offset++;
	url[2].latitude = strtod(value,NULL);
	url[2].longitude = strtod(offset,NULL);
	
	/* Quit curl */
	curl_easy_cleanup(curl);
	

	/*---------------- PROCESS AND DISPLAY RESULTS ----------------*/
	/* Difference between latitude lines is 60 nautical miles, or 111.12 kilometers.
	   For longitude, the value changes as it gets closer to the poles:
	   1 deg of longitude = cos(latitude)*111 kilometers at the equator.
	   Values are approximate because Earth isn't a perfect sphere. */
	/* My method here is to average all the latitude and longitude values, then
	   determine the farthest distance from that average. I use this farthest
	   value (latitude or longitude) to set the accuracy radius in kilometers.
	   Yes, this may not be a cartographically proper solution, but all this data
	   is guesswork anyway. */

	/* Obtain the average latitude and longitude values */
	tot_lon = tot_lat = 0.0;
	for(x=0;x<3;x++)
	{
		tot_lat+=url[x].latitude;
		tot_lon+=url[x].longitude;
	}
	avg_lat = tot_lat/3.0;
	avg_lon = tot_lon/3.0;

	/* Check which longitude & latitude items are farthest from the average */
	max_lon = max_lat = 0.0;
	for(x=0;x<3;x++)
	{
		max_lon = max_lon < fabs(avg_lon)-fabs(url[x].longitude) ? fabs(avg_lon)-fabs(url[x].longitude) : max_lon;
		max_lat = max_lat < fabs(avg_lat)-fabs(url[x].latitude) ? fabs(avg_lat)-fabs(url[x].latitude) : max_lat;
	}

	/* Set the distance of max_lon and max_lat in kilometers
	   and set the maximum as the radius value. Degrees-to-radians
	   calculation uses 0.0174532925 */
	rad_lon = max_lon*cos(avg_lat*0.0174532925)*111.12;
	rad_lat = max_lat*111.12;
	radius = rad_lon >= rad_lat ? rad_lon : rad_lat;

	/* Display the Results */
	printf("Data for IP address %s:\n",ip);
	printf("%s\t%s\t%s\n",
			"Site",
			"Latitude",
			"Longitude"
		  );
	for(x=0;x<3;x++)
	{
		printf("%3d\t%f\t%f\n",
				x+1,
				url[x].latitude,
				url[x].longitude
			  );
	}
	printf("Accuracy is within a radius of %f kilometers\n",radius);

	return(0);
}


/*
 * This is a routine to fetch key-value details in JSON data.  
 * It is handy here for this specific purpose only. This code
 * shouldn't be used to parse other JSON structures.
 */
void parse(char *json, char *key, char *val)
{
	char *found,*size;
	int x;

	/* Locate the string and add its length, plus one for the double quote */
	found = strstr(json,key)+1;
	/* Find the colon */
	while(*found!=':')
	{
		if(*found=='\0')
		{
			fprintf(stderr,"Unable to parse value for '%s'\n",key);
			exit(1);
		}
		found++;
	}
	/* Find the next character after the second double quote */
	while(*found!='\"')
	{
		if(*found=='\0')
		{
			fprintf(stderr,"Unable to parse value for '%s'\n",key);
			exit(1);
		}
		found++;
	}
	/* And skip past the double quote */
	found++;

	/* Find the end of the value */
	size = found+1;
	while(*size != '\"')
	{
		if(*size=='\0')
		{
			fprintf(stderr,"Unable to parse value for '%s'\n",key);
			exit(1);
		}
		size++;
	}

	/* Copy the string */
	x = 0;
	*val='\0';
	while(*(found+x) != '\"')
	{
		if(*(found+x)=='\0')
		{
			fprintf(stderr,"Malformed json value\n");
			exit(1);
		}
		*(val+x) = *(found+x);
		x++;
	}
	/* Cap with a null character */
	*(val+x) = '\0';
}


/*
 * Previous "parse()" is unable to process values without double quotes.
 * Modified previous parse() to extract values from JSON data which 
 * doesn't use double quotes.
 */
void extractValue(char *json, char *key, char *val)
{
	char *found,*size;
	int x;

	/* Locate the string and add its length, plus one for the double quote */
	found = strstr(json,key)+1;
	/* Find the colon */
	while(*found!=':')
	{
		if(*found=='\0')
		{
			fprintf(stderr,"[:] Unable to parse value for '%s'\n",key);
			exit(1);
		}
		found++;
	}
	/* Find the next digit character after the colon */
	while(!(isdigit(*found)))
	{
		if(*found=='\0')
		{
			fprintf(stderr,"[digit] Unable to parse value for '%s'\n",key);
			exit(1);
		}
		found++;
	}

	/* Find the end of the value */
	size = found+1;
	while( *size!=',' && *size!=' ' && *size!='}' )
	{
		if(*size=='\0')
		{
			fprintf(stderr,"[EOF] Unable to parse value for '%s'\n",key);
			exit(1);
		}
		size++;
	}

	/* Copy the string */
	x = 0;
	*val='\0';
	while( *(found+x)!=',' && *(found+x)!=' ' && *(found+x)!='}' )
	{
		if(*(found+x)=='\0')
		{
			fprintf(stderr,"Malformed json value\n");
			exit(1);
		}
		*(val+x) = *(found+x);
		x++;
	}
	/* Cap with a null character */
	*(val+x) = '\0';
}


/*
 * Return a specific item from the csv string
 * v is the item number to read, from 1 up to whatever. The csv string
 * is referenced by *csv. *f is the buffer to contain the found data.
 * Data is created locally in buffer[], then copied to *f.
 * Modified so that the result value isn't returned and if the field
 * isn't available, the program quits within the function.
 */
void fetch(int v, char *csv, char *f)
{
	char buffer[BUFSIZE];
	char *b,*cptr;
	int bi,count;

	/* Ensure that v is valid */
	if(v<1)
	{
		fprintf(stderr,"Inavlid field for CSV string\n");
		exit(1);
	}

	/* Scan for valid fields and pull them out */
	cptr = csv;
	b = buffer;
	bi = 0;
	count = 1;
	while(*cptr)
	{
		/* Start copying characters */
		*(b+bi) = *(cptr);
		
		/* If a quoted string is encountered, copy it all over */
		if(*(b+bi)=='"')
		{
			/* Skip over the comma as it's not really part of the string */
			/* Do this by resetting bi to -1, which then increments to zero
			   with the next statement */
			bi = -1;
			do
			{
				bi++;
				cptr++;
				if(bi>BUFSIZE)
				{
					fprintf(stderr,"Malformed CSV field\n");
					exit(1);
				}
				*(b+bi) = *(cptr);
			} while(*(b+bi)!='"');
			/* Skip over the final double quote */
			cptr++;
		}
		/* When the comma is encountered, a field has ended */
		if(*cptr==',')
		{
			if(count==v)
			{
				/* Cap the string */
				*(b+bi) = '\0';
				strcpy(f,buffer);
				return;
			}
			else
			{
				/* Up the count and reset bi */
				count++;
				bi=-1;
			}
		}
		bi++;
		cptr++;
		/* Check for buffer overflow */
		if(bi>BUFSIZE)
			break;
	}
	/* Check to see whether it's the final item */
	if(count==v)
	{
		*(b+(bi))='\0';
		strcpy(f,buffer);
		return;
	}

	/* If we get here, there was some sort of error */
	fprintf(stderr,"Unable to read field %d from CSV record\n",v);
	exit(1);
}


/*
 * Libcurl function to copy bytes read from the web page to memory
 * This is the typical memory-writing function used in curl coding.
 * Information is read in chunks and appended to a buffer. This function
 * may be called repeatedly, which is why it's static, to retain the
 * web_data structure's buffer and size values.
 */
static size_t write_mem(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize;
	struct web_data *mem;

	realsize = size * nmemb;
	mem = (struct web_data *)userdata;

	mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);
	if(mem->buffer == NULL)
	{
		fprintf(stderr,"Unable to reallocate buffer\n");
		exit(1);
	}

	memcpy(&(mem->buffer[mem->size]),ptr,realsize);
	mem->size += realsize;
	mem->buffer[mem->size] = 0;

	return(realsize);
}
