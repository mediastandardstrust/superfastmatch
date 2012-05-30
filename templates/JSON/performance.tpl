{{#INSTRUMENT_GROUP}}
    "{{NAME}}"     :{
                    "metaData"  :{
                                    "fields"      : [{{#FIELDS}}"{{FIELD:o}}"{{#FIELDS_separator}},{{/FIELDS_separator}}{{/FIELDS}}]
                    },
                    "rows"      :[
                                    {{#INSTRUMENT}}
                                    {
                                      {{#META}}
                                      "{{KEY:o}}": {{#NUMBER}}{{VALUE}}{{/NUMBER}}{{#STRING}}"{{VALUE:o}}"{{/STRING}}{{#META_separator}},{{/META_separator}}
                                      {{/META}}
                                    }{{#INSTRUMENT_separator}},{{/INSTRUMENT_separator}}
                                    {{/INSTRUMENT}}
                                 ]
                    }{{#INSTRUMENT_GROUP_separator}},{{/INSTRUMENT_GROUP_separator}}                 
{{/INSTRUMENT_GROUP}}


