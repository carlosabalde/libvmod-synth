
.. image:: https://travis-ci.org/carlosabalde/libvmod-synth.svg?branch=5.x
   :alt: Travis CI badge
   :target: https://travis-ci.org/carlosabalde/libvmod-synth/

Simple VMOD useful to generate synthetic responses during the ``vcl_synth`` or ``vcl_backend_error`` phases. Four types of responses are supported:

* **Files**: delivers contents of any readable file, including binary ones.
* **Templates**: delivers contents of any readable template file, once rendered according with a list of tuples representing names and values of placeholders.
* **Pixels**: delivers transparent 1px GIF images.
* **Strings**: delivers any string value (i.e. no difference at all with the ``synthetic()`` primitive).

SYNOPSIS
========

import synth;

::

    Function VOID file(STRING)
    Function VOID template(STRING, STRING, STRING)
    Function VOID pixel()
    Function VOID string(STRING)

INSTALLATION
============

The source tree is based on autotools to configure the building, and does also have the necessary bits in place to do functional unit tests using the varnishtest tool.

COPYRIGHT
=========

See LICENSE for details.

* Copyright (c) 2014-2015 Carlos Abalde <carlos.abalde@gmail.com>
