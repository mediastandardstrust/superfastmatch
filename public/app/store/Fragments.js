Ext.define('Superfastmatch.store.Fragments', {
    extend: 'Ext.data.Store',
    fields: ['from','to','length','fullText','text',{name:'count',defaultValue:1}],
    
    loadDocuments: function(docs){
        var me=this;
        me.removeAll();
        Ext.each(docs,function(doc){
            doc.fragments().each(function(fragment){
                var from=fragment.get('from'),
                    to=fragment.get('to'),
                    length=fragment.get('length'),
                    text=fragment.getText(),
                    record=me.getAt(me.findExact('fullText',text));
                    // record=me.getAt(me.find('fullText',text,null,false));
                    // record=me.getAt(me.findBy(function(record,id){
                    //     return ((record.get('from')==from)||(record.get('to')==to)) && record.get('length')==length;
                    // }));
                if(record){
                    record.set('count',record.get('count')+1);
                }else{
                    me.add({
                        from: from,
                        to: to,
                        length: length,
                        fullText: text,
                        text: Ext.String.ellipsis(text,500,true)
                    });   
                }
            });
        });
        this.sync();
        this.sort();
    }
});