{{>HEADER}}
	<h1>Documents</h1>
	<table>
		<caption>
			{{#FIRST}}<a href="{{PAGE}}">First Page</a>{{/FIRST}}
			{{#PREVIOUS}}<a href="{{PAGE}}">Previous Page</a>{{/PREVIOUS}}
			{{#NEXT}}<a href="{{PAGE}}">Next Page</a>{{/NEXT}}
			{{#LAST}}<a href="{{PAGE}}">Last Page</a>{{/LAST}}
		</caption>
		<thead>
			<th>
				Document Type
			</th>
			<th>
				Document ID
			</th>
			{{#KEYS}}
			<th>
				{{KEY}}
			</th>
			{{/KEYS}}
		</thead>
		<tbody>
			{{#DOCUMENT}}
			<tr>
				<td class="line"><a href="/document/{{DOC_TYPE}}/">{{DOC_TYPE}}</a></td>
				<td class="line"><a href="/document/{{DOC_TYPE}}/{{DOC_ID}}/">{{DOC_ID}}</a></td>
				{{#VALUES}}
				<td class="line">
					{{VALUE}}
				</td class="line">
				{{/VALUES}}
			</tr>
			{{/DOCUMENT}}
		</tbody>
	</table>
{{>FOOTER}}