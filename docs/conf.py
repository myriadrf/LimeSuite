# Configuration file for the Sphinx documentation builder.
#
# MyriadRF conf.py v1.2.0

import sys
import sphinx_rtd_theme
import requests
import warnings
from pathlib import Path
from urllib.parse import urljoin
import json

# -- Setup and helper functions------------------------------------------------

# Path to remote assets
asset_base = 'https://assets.myriadrf.net/sphinx/'

# Put config directory first on sys.path so imports from this work
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

# Import the project config from project.py
import project as project_cfg

# Retrieve remote asset
def _fetch_remote(path, base=asset_base, timeout=10, default=""):
    url = urljoin(base, path)
    try:
        resp = requests.get(url, timeout=timeout)
        resp.raise_for_status()
        return resp.text
    except requests.RequestException as exc:
        warnings.warn(f"Failed to fetch {url}: {exc}")
        return default
    
# Read local asset 
def _read_local(path: Path) -> str:
    try:
        return path.read_text(encoding='utf-8')
    except Exception:
        return ""

# -- Basic configuration ------------------------------------------------------

# Sphinx extensions
extensions = [
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.intersphinx',
    'sphinx.ext.mathjax',
    'sphinx-mathjax-offline',
    'sphinx_code_tabs',
    'sphinx_rtd_theme',
    'notfound.extension',
    'myst_parser'
]

# Allow same section headings and thus labels to be used across documents.
autosectionlabel_prefix_document = True

# Paths that contain templates
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['node_modules', 'venv', '_build', 'Thumbs.db', '.DS_Store']

# -- Project config and MyriadRF styling --------------------------------------

# Assign project config variables to local names
project = project_cfg.project 
copyright = project_cfg.copyright
author  = project_cfg.author
release = project_cfg.release

# Set the CSS to use
if not project_cfg.archived:
    html_css_files = [urljoin(asset_base, 'mr-custom.css')]
else:
    html_css_files = [urljoin(asset_base, 'mr-archived.css')]

# Get custom footer and navbar
extrabody_content = _fetch_remote('mr-navbar.html')
extrafooter_content = _fetch_remote('mr-footer.html')

# HTML customisation 
html_context = {
    'extrabody': extrabody_content,
    'extrafooter': extrafooter_content,
    'display_github': True,
    'github_user': 'myriadrf',
    'github_repo': project_cfg.github_repo,
    'github_version': project_cfg.github_repo_path,
    'archived': project_cfg.archived
}

# -- RST epilog (used to define project/global links and substitutions) -------

# Get remote global substitutions and external links
global_subs = _fetch_remote('substitutions.conf')
global_extlinks = _fetch_remote('extlinks.conf')

# Get local project-wide substitutions and external links
local_subs = _read_local(HERE / 'substitutions.conf')
local_extlinks = _read_local(HERE / 'extlinks.conf')

# Merge remote + local RST epilog content
parts = []

if global_subs:
    parts.append(global_subs.rstrip() + "\n")
if local_subs:
    parts.append(local_subs.rstrip() + "\n")
if global_extlinks:
    parts.append(global_extlinks.rstrip() + "\n")
if local_extlinks:
    parts.append(local_extlinks.rstrip() + "\n")

if parts:
    rst_epilog = "\n".join(parts)
else:
    # Fallback: if nothing was found/read, keep original include-style epilog 
    # so that Sphinx can try to include files by path.
    rst_epilog = """
.. include:: /substitutions.conf
.. include:: /extlinks.conf
"""

# -- Intersphinx configuration ------------------------------------------------

intersphinx_mapping = {}

# Get the global intersphinx mappings
global_intersphinx = _fetch_remote('intersphinx.json')

# Parse JSON and extract internal and external maps
intersphinx_data = {}

if global_intersphinx:
    try:
        intersphinx_data = json.loads(global_intersphinx)
    except Exception as exc:
        warnings.warn(f"Failed to parse intersphinx.json from remote assets host: {exc}")
        intersphinx_data = {}

internal_map = {}
external_map = {}

if isinstance(intersphinx_data, dict):
    internal_map = intersphinx_data.get('internal', {}) or {}
    external_map = intersphinx_data.get('external', {}) or {}

# Determine MyriadRF projects host based on staging flag (default False)
use_staging = bool(getattr(project_cfg, 'staging', False))
host = 'stage.myriadrf.org' if use_staging else 'myriadrf.org'

# Build intersphinx_mapping according to project.py configuration

# Process MyriadRF project list
intersphinx_internal = getattr(project_cfg, 'intersphinx_internal', None)
if isinstance(intersphinx_internal, (list, tuple)):
    for name in intersphinx_internal:
        if not isinstance(name, str) or not name:
            continue
        slug = internal_map.get(name)
        if slug:
            intersphinx_mapping[name] = (f'https://{host}/projects/{slug}/', None)
        else:
            warnings.warn(f"Intersphinx mapping for MyriadRF project '{name}' not found in intersphinx.json; skipping.")

# Process external project list
intersphinx_external = getattr(project_cfg, 'intersphinx_external', None)
if isinstance(intersphinx_external, (list, tuple)):
    for name in intersphinx_external:
        if not isinstance(name, str) or not name:
            continue
        url = external_map.get(name)
        if url:
            intersphinx_mapping[name] = (url, None)
        else:
            warnings.warn(f"Intersphinx mapping for external project '{name}' not found in intersphinx.json; skipping.")

# -- Options for HTML output --------------------------------------------------

html_theme = "sphinx_rtd_theme"

html_theme_options = {
    'logo_only': False,
    'collapse_navigation': False,
    'sticky_navigation': True,
    'navigation_depth': 5
}

html_show_sphinx = False

# HTML last updated formatting
html_last_updated_fmt = '%b %d, %Y'

root_doc = 'index'

