RISC Imposter
########################################
A general purpose cycle accurate RISC cpu simulator.

This simulator currenly supports MIPS arch. It is designed to support other CPU arches as well.


Instruction Support:
====================

MIPS:

* R type: Add, And, Or, Xor, Slt
* I type: Beq, Lw, Sw
* J type: j



Dependencies
============

Imposter needs meson build system and a sane C compiler.



macOS
-----

Imposter's dependencies can be installed via `Homebrew <http://brew.sh/>`_
:

.. code-block:: bash

    $ brew install clang meson



Linux
-----


You need Clang or gcc, plus meson.

They can be installed using the system package manager, for example:

.. code-block:: bash

    # Debian, Ubuntu, etc.
    $ apt-get install meson clang

.. code-block:: bash

    # Fedora
    $ dnf install meson clang

.. code-block:: bash

    # CentOS, RHEL, ...
    $ yum install meson clang

.. code-block:: bash

    # Arch Linux
    $ pacman -S meson clang



Windows:
--------


First, ensure that you have a working C compiler.

Then install `meson <http://mesonbuild.com/>`_



Compiling
=========

Go to the root of the project and run:



.. code-block:: bash

    $ meson build
    $ cd build/
    $ ninja



Usage
=====


.. code-block:: bash

    $ ./imposter



Contribution:
============

You know the drill:

* fork
* new feature branch
* do stuff
* commit
* push
* pull request

