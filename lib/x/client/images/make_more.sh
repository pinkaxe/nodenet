#!/bin/bash

# create more images for testing uploading lots of files

i=0
while((++i <= 1200));do
	cp lips.png lips.png$i;
done;
