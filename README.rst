==========
vmod_synth
==========

--------------------
Varnish Synth Module
--------------------

:Author: Carlos Abalde
:Date: 2014-07-20
:Version: 0.1
:Manual section: 3

SYNOPSIS
========

import synth;

DESCRIPTION
===========

Simple VMOD useful to generate synthetic responses during the `vcl_error` phase. Four types of responses are supported:

* Files: delivers contents of any readable file, including binary ones.
* Templates: delivers contents of a template file once rendered according with a list of tuples representing placeholders names and values.
* Pixels: delivers 1px transparent GIF images.
* Strings: delivers any string value (i.e. no difference at all with the `synthetic()` primitive).

FUNCTIONS
=========

file
----

Prototype
        ::

                file(STRING path)
Arguments
    path: location of a file readable by Varnish Cache.
Return value
    VOID
Description
    Does the same as synthetic() in VCL, but uses the contents of a file. Must be used in vcl_error.
    Beware that files are internally cached for further usage.
    Cached files can be updated simply reloading the VCL.
Example
        ::

            sub vcl_error {
                if (obj.status == 700) {
                    synth.file("/var/www/logo.png");

                    set obj.status = 200;
                    set obj.response = "OK";
                    set obj.http.Content-Type = "image/png";

                    return (deliver);
                }
            }

template
--------

Prototype
        ::

                template(STRING path, STRING placeholders)
Arguments
    path: location of a file readable by Varnish Cache.

    placeholders: pipe (|) delimited list of placeholders names & values.
Return value
    VOID
Description
    Does the same as synthetic() in VCL, but uses the contents of a template file.
    Must be used in vcl_error.
    Beware that template files are internally cached for further usage.
    Cached template files can be updated simply reloading the VCL.
Example
        ::

            sub vcl_error {
                synth.file(
                    "/var/www/error.html",
                    "{{ xid }}|" + req.xid + "|" +
                    "{{ status }}|" + obj.status);

                set obj.response = "OK";
                set obj.http.Content-Type = "text/html";
                set obj.status = 200;

                return (deliver);
            }

pixel
-----

Prototype
        ::

                pixel()
Return value
    VOID
Description
    Does the same as synthetic() in VCL, but uses the contents of a 1px transparent GIF image.
    Must be used in vcl_error.
Example
        ::

            sub vcl_error {
                if (obj.status == 700) {
                    std.log("...");

                    synth.pixel();

                    set obj.status = 200;
                    set obj.response = "OK";
                    set obj.http.Content-Type = "image/gif";

                    return (deliver);
                }
            }

string
------

Prototype
        ::

                string(STRING value)
Arguments
    value: any string value.
Return value
    VOID
Description
    Does the same as synthetic() in VCL.
    Must be used in vcl_error.
Example
        ::

            sub vcl_error {
                synth.string("Hello world!");

                set obj.status = 200;
                set obj.response = "OK";
                set obj.http.Content-Type = "text/plain";

                return (deliver);
            }

INSTALLATION
============

The source tree is based on autotools to configure the building, and does also have the necessary bits in place to do functional unit tests using the varnishtest tool.

Usage::

 ./configure VARNISHSRC=DIR [VMODDIR=DIR]

`VARNISHSRC` is the directory of the Varnish source tree for which to compile your VMOD. Both the `VARNISHSRC` and `VARNISHSRC/include` will be added to the include search paths for your module.

Optionally you can also set the VMOD install directory by adding `VMODDIR=DIR` (defaults to the pkg-config discovered directory from your Varnish installation).

Make targets:

* make - builds the VMOD
* make install - installs your VMOD in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

COPYRIGHT
=========

This document is licensed under the same license as the libvmod-synth project. See LICENSE for details.

* Copyright (c) 2014 Carlos Abalde <carlos.abalde@gmail.com>
