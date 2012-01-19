{{%AUTOESCAPE context="JSON"}}
        "metaData"  :{
                        "fields"      : [{{#FIELDS}}"{{FIELD}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
        },
        "rows"      :[
                        {{#DOCUMENT}}
                        {
                            {{#STRING}}
                            "{{KEY}}":"{{VALUE}}",
                            {{/STRING}}
                            {{#NUMBER}}
                            "{{KEY}}":{{VALUE}},
                            {{/NUMBER}}
                            {{#DATE}}
                            "{{KEY}}":new Date({{VALUE}}),
                            {{/DATE}}
                            {{#FRAGMENTS}}
                            "fragments" : [{{#FRAGMENT}}[{{FROM}},{{TO}},{{LENGTH}},{{HASH}}]{{#FRAGMENT_separator}},{{/FRAGMENT_separator}}{{/FRAGMENT}}]
                            {{/FRAGMENTS}}
                        }{{#DOCUMENT_separator}},{{/DOCUMENT_separator}}
                        {{/DOCUMENT}}
                     ]
