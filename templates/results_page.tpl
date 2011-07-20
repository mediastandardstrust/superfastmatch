{{>HEADER}}
	<h1>Search Results</h1>
	<ul>
	{{#RESULT}}
		<li>{{DOC_TYPE}} {{DOC_ID}} {{COUNT}} {{TOTAL}} {{HEAT}}</li>
	{{/RESULT}}
	</ul>
	{{>ASSOCIATION}}
{{>FOOTER}}