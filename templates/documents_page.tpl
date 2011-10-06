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
				<a href="?order_by=doctype">Document Type</a>
			</th>
			<th>
				<a href="?order_by=docid">Document ID</a>
			</th>
			{{#KEYS}}
			<th>
				<a href="?order_by={{DIRECTION}}{{KEY}}">{{KEY}}</a>
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