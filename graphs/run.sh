#!/bin/sh
java -jar plantuml.jar -tsvg seq.uml
java -jar plantuml.jar -tsvg slavestate.uml

#inkview seq.svg
