# Change Log

## 2026-02-25

* Problems with the 8K linear CODEC have been addressed.
* The accessibility of the user interface has been improved. (Thanks
to Joe KA9OPL for his help on this)
* Node statistics have been enabled. You should be able to see your node
using the [ASL stats page like this](https://stats.allstarlink.org/stats/672733).

## 2026-02-22

* Removed support for SLIN8, having problems.
* Improved UI accessibility, particularly the PTT button. Thanks to Joe KA9OPL.

## 2026-02-16

* The --httppwd command-line option was added to allow a password to be set
for the user interface. By default there is no authentication required. Thanks to 
Smitty WB1G for this suggestion.
* Internal changes have been made to improve performance and scalability.
* Problems with CODEC negotiation have been addressed.
* Support for the G.726 AAL2 CODEC has been added.

## 2026-02-09

* Now statically linking the CA Certs file downloaded from 
the Curl website to simplify deployment. Please 
see https://curl.se/docs/caextract.html.
* Took latest amp-core with multiple performance improvements.
