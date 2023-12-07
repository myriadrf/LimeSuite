Documentation generation guide
==============================

This page describes how to generate the LimeSuiteGUI2 documentation
(the very thing you're reading right now) on Linux.

Prerequisites
-------------

Components needed to generate the LimeSuiteGUI2 documentation:

- `GCC`_
- `CMake`_
- `Python 3.8`_
- `Doxygen`_

Setup
-----

To set up the environment for the documentation generation, you will need to set up a `Python virtual environment (venv)`_.

To set up the environment, run these commands:

.. note::
    One should run these commands in the `docs` folder.

.. code-block:: bash

    python3.8 -m venv venv # Create the virtual environment folder
    source venv/bin/activate # Activate the virtual environment
    pip install -r requirements.txt # Install all the required dependencies for the generation

Generation
----------

In the `docs` folder, located in the root folder of the repository, while in the venv, run these commands:

.. important:: 
    These commands or the script must be run in the venv, otherwise it will fail.

.. code-block:: bash

    cmake -S .. -B ../build # Generate the make file for the suite.
    make --no-print-directory -C ../build doc # Build Doxygen documentation
    breathe-apidoc --generate class --members --force --output-dir apidoc ../build/xml/ # Generate the class API pages
    make html # Generate the documentation itself

For ease of convenience, there is also a `generate_docs.sh` script located in the `docs` folder.

After a successful generation the resulting documentation pages will be located in 
`docs/_build/html` folder.

Docker
------

There also exists a Docker file to generate the documentation.

Docker prerequisites
^^^^^^^^^^^^^^^^^^^^

In order to use the Docker documentation generation file, you will need to have the `Docker Engine`_ installed and set up.

Running the Docker documentation generation file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To generate this documentation using Docker, in the root folder of the repository run:

.. code-block:: bash

    docker build -o <output_location> -f GenerateDocumentation.Dockerfile .

This will run the whole documentation generation script and place the generated HTML in the specified folder.

More information
----------------

For more information about how to set up and write the documentation,
check out the `MyriadRF Handbook`_.

.. _`GCC`: https://gcc.gnu.org/
.. _`CMake`: https://cmake.org/
.. _`Python 3.8`: https://www.python.org/downloads/release/python-3818/
.. _`Doxygen`: https://www.doxygen.nl/
.. _`Python virtual environment (venv)`: https://docs.python.org/3.8/library/venv.html
.. _`MyriadRF Handbook`: https://handbook.myriadrf.org/
.. _`Docker Engine`: https://docs.docker.com/engine/