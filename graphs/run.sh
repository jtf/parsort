#!/bin/sh
java -jar plantuml.jar -tsvg seq.uml

inkview seq.svg
