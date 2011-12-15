Ext.define('Superfastmatch.model.Fragment', {
    extend: 'Ext.data.Model',
    fields: ['from','to','length','hash'],
    associations: [
        { type: 'belongsTo', model: 'Superfastmatch.model.Document',getterName: 'getDocument' }
    ],
    getText: function(){
        return this.getDocument().getSearch().get('text').substr(this.get('from'),this.get('length'));
    }
});