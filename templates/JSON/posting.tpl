    "total" : {{TOTAL}},
    "cursors": {    
                    "current"   : "{{CURRENT:o}}",
                    "first"     : "{{FIRST:o}}",
                    "last"      : "{{LAST:o}}",
                    "previous"  : "{{PREVIOUS:o}}",
                    "next"      : "{{NEXT:o}}"
    },
    "rows"      :[
                    {{#HASH}}
                    {
                      "hash"      : {{HASH}},
                      "bytes"     : {{BYTES}},
                      "doctypes"  : [{
                                      {{#POSTING}}
                                        "doctype"   : {{DOC_TYPE}},
                                        "bytes"     : {{BYTES}},
                                        "docids"    : [{{#DOC_IDS}}{{DOC_ID}}{{#DOC_IDS_separator}},{{/DOC_IDS_separator}}{{/DOC_IDS}}],
                                        "deltas"    : [{{#DOC_DELTAS}}{{DOC_DELTA}}{{#DOC_DELTAS_separator}},{{/DOC_DELTAS_separator}}{{/DOC_DELTAS}}]
                                     }{{#POSTING_separator}},{{/POSTING_separator}}
                                      {{/POSTING}}
                                    ]
                    }
                    {{#HASH_separator}},{{/HASH_separator}}
                    {{/HASH}}
                 ]
