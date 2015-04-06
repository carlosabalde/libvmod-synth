==========
vmod_synth
==========

.. image:: https://travis-ci.org/carlosabalde/libvmod-synth.svg?branch=master
    :target: https://travis-ci.org/carlosabalde/libvmod-synth

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

Simple VMOD useful to generate synthetic responses during the ``vcl_synth`` or ``vcl_backend_error`` phases. Four types of responses are supported:

* **Files**: delivers contents of any readable file, including binary ones.
* **Templates**: delivers contents of any readable template file, once rendered according with a list of tuples representing names and values of placeholders.
* **Pixels**: delivers transparent 1px GIF images.
* **Strings**: delivers any string value (i.e. no difference at all with the ``synthetic()`` primitive).

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
    Does the same as the ``synthetic()`` primitive, but uses the contents of a file.
    Must be used during the ``vcl_synth`` or ``vcl_backend_error`` phases.
    Beware that files are internally cached for further usage.
    Cached files can be updated simply reloading the VCL.
Example
        ::

            sub vcl_synth {
                if (resp.status == 700) {
                    synth.file("/etc/varnish/assets/logo.png");

                    set resp.status = 200;
                    set resp.reason = "OK";
                    set resp.http.Content-Type = "image/png";

                    return (deliver);
                }
            }

template
--------

Prototype
        ::

                template(STRING path, STRING placeholders, STRING delimiter)
Arguments
    path: location of a template file readable by Varnish Cache.

    placeholders: delimited list of placeholders names and values.

    delimiter: character delimiting names and values in the list of placeholders.
Return value
    VOID
Description
    Does the same as the ``synthetic()`` primitive, but uses the contents of a template file.
    Must be used during the ``vcl_synth`` or ``vcl_backend_error`` phases.
    Beware that template files are internally cached for further usage.
    Cached template files can be updated simply reloading the VCL.
Example
        ::

            sub vcl_synth {
                synth.file(
                    "/etc/varnish/templates/error.html",
                    "{{ xid }}|" + req.xid + "|{{ status }}|" + resp.status,
                    "|");

                set resp.reason = "OK";
                set resp.http.Content-Type = "text/html";
                set resp.status = 200;

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
    Does the same as the ``synthetic()`` primitive, but uses the contents of a transparent 1px GIF image.
    Must be used during the ``vcl_synth`` or ``vcl_backend_error`` phases.
Example
        ::

            sub vcl_synth {
                if (resp.status == 700) {
                    std.log("...");

                    synth.pixel();

                    set resp.status = 200;
                    set resp.reason = "OK";
                    set resp.http.Content-Type = "image/gif";

                    set obj.http.Cache-Control = "no-cache, no-store, must-revalidate";
                    set obj.http.Pragma = "no-cache";
                    set obj.http.Expires = "0";

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
    Does the same as the ``synthetic()`` primitive.
    Must be used during the ``vcl_synth`` or ``vcl_backend_error`` phases.
Example
        ::

            sub vcl_synth {
                synth.string("Hello world!");

                set resp.status = 200;
                set resp.reason = "OK";
                set resp.http.Content-Type = "text/plain";

                return (deliver);
            }

INSTALLATION
============

The source tree is based on autotools to configure the building, and does also have the necessary bits in place to do functional unit tests using the varnishtest tool.

COPYRIGHT
=========

See LICENSE for details.

* Copyright (c) 2014-2015 Carlos Abalde <carlos.abalde@gmail.com>
