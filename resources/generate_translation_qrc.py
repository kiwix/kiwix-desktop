#!/usr/bin/env python3
# Copyright (c) 2020 Matthieu Gautier <mgautier@kymeria.fr>
#
# This file is part of kiwix-desktop.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from pathlib import Path
import xml.etree.ElementTree as ET
import xml.etree

script_path = Path(__file__)

out_qrc_file = script_path.parent / "translations.qrc"
translation_dir = script_path.parent / "i18n"

root = ET.Element("RCC")
root.text = "\n    "
qresource = ET.SubElement(root, "qresource")
qresource.set("prefix", "/")
qresource.text = "\n" + " " * 8
qresource.tail = "\n"
json_files = translation_dir.glob("*.json")

node = None
for json in sorted(translation_dir.glob("*.json")):
    if node is not None:
        node.tail += " " * 8
    node = ET.SubElement(qresource, "file")
    node.text = str(json.relative_to(script_path.parent))
    node.tail = "\n"

node.tail += " "*4

out_qrc_file.write_bytes(ET.tostring(root, encoding='utf8'))
