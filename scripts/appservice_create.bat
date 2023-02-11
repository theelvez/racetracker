#!/bin/bash

# create an Azure App Service named "speedtracker"

az appservice web create \
--resource-group "screamingelfresources" \
--plan "screamingelfappserviceplan" \
--name "speedtracker" \
--deployment-local-git

# upload all the code to the appservice web app

cd /home/site/wwwroot

git init
git add .
git commit -m "initial commit"

az appservice web source-control config-local-git \
--name "speedtracker" \
--resource-group "screamingelfresources" \
--query url --output tsv

git remote add azure <url>

git push azure master
