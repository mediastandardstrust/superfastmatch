{{>HEADER}}
	<h1>Status</h1>
	<h2>Index:</h2>
	<ul>
		<li>{{HASH_WIDTH}} bits per hash</li>
		<li>{{SLOT_COUNT}} slots in index</li>
		<li>{{DOC_COUNT}} docs in index</li>
		<li>{{HASH_COUNT}} hashes added to index</li>
		<li>{{AVERAGE_DOC_LENGTH}} average document length</li>	
		<li><a href="/index/?cursor={{WHITESPACE_HASH}}">Whitespace</a></li>
	</ul>
	<h2>Total Memory Usage: {{MEMORY}} GB</h2>
	{{#DB}}
	<h2>{{NAME}}</h2>
	<pre>{{STATS}}</pre>
	{{/DB}}		
	<h2>Memory:</h2>
	<pre>{{MEMORY_STATS}}</pre>
{{>FOOTER}}