#!/bin/sh

script/bootstrap
bundle install
bundle exec rake
