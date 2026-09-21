#!/usr/bin/env python3
from pathlib import Path
from collections import Counter
import os
import re
import xml.etree.ElementTree as ET
sdk = Path(__file__).resolve().parents[1]
projects = {"xovi-extension-manager-ui":"xovi-extension-manager-ui", "advanced_settings":"advanced_settings",
            "keyboardcjk":"keyboardcjk", "epub-preloader":"epub-preloader", "rm-librarian":"rm-librarian",
            "rmfakecloud-control":"xovi-rmfakecloud-plugin/plugin"}
for plugin, path in projects.items():
    directory = Path(os.environ.get("XOVI_WORKSPACE", sdk.parent)) / path / "translations"
    reference = None
    for language in ('en', 'zh_CN', 'zh_TW'):
        root = ET.parse(directory / f'{plugin}_{language}.ts').getroot()
        keys = set()
        for context in root.findall('context'):
            for message in context.findall('message'):
                source = message.findtext('source')
                key = (context.findtext('name'), source)
                assert key not in keys, f'duplicate message: {key}'
                keys.add(key)
                translated = message.find('translation')
                assert translated is not None and translated.text and not translated.get('type'), (language, key)
                assert Counter(re.findall(r'%[1-9n]', source)) == Counter(re.findall(r'%[1-9n]', translated.text)), (language, key)
                if language == 'en':
                    assert not re.search(r'[\u4e00-\u9fff]', translated.text), ('Chinese left in English catalog', key)
        if reference is None:
            reference = keys
        assert keys == reference, f'{language}: catalog coverage differs'
        print(f'PASS {language}: {len(keys)} complete messages and valid placeholders')
