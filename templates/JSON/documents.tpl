{{%AUTOESCAPE context="JSON"}}
        "metaData"  :{
                        "fields"      : [{{#FIELDS}}"{{FIELD}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
        },
        "rows"      :[
                        {{#DOCUMENT}}
                        {
                          {{#FRAGMENTS}}
                          "fragments" : [{{#FRAGMENT}}[{{FROM}},{{TO}},{{LENGTH}},{{HASH}}]{{#FRAGMENT_separator}},{{/FRAGMENT_separator}}{{/FRAGMENT}}],
                          {{/FRAGMENTS}}
                          {{#META}}
                          "{{KEY}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE}}"{{/STRING}}{{#DATE}}new Date({{VALUE}}){{/DATE}}{{#META_separator}},{{/META_separator}}
                          {{/META}}
                        }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}
                        {{/DOCUMENT}}
                     ]
