/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * GData Client
 * Copyright (C) Philip Withnall 2008–2010 <philip@tecnocode.co.uk>
 *
 * GData Client is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GData Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GData Client.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <locale.h>

#include "gdata.h"
#include "common.h"

static void
test_entry_get_xml (void)
{
	GTimeVal updated, published, updated2, published2, *updated3, *published3;
	GDataEntry *entry, *entry2;
	GDataCategory *category;
	GDataLink *_link; /* stupid unistd.h */
	GDataAuthor *author;
	gchar *xml, *title, *summary, *id, *etag, *content, *content_uri, *rights;
	gboolean is_inserted;
	GList *list;
	GError *error = NULL;

	entry = gdata_entry_new (NULL);

	/* Test setting properties directly */
	g_object_set (G_OBJECT (entry),
	              "title", "First testing title & escaping",
	              "summary", "Test summary & escaping.",
	              "content", "Test <markup> & escaping.",
	              "rights", "Philip Withnall <philip@tecnocode.co.uk>",
	              NULL);

	g_assert_cmpstr (gdata_entry_get_title (entry), ==, "First testing title & escaping");
	g_assert_cmpstr (gdata_entry_get_summary (entry), ==, "Test summary & escaping.");
	g_assert_cmpstr (gdata_entry_get_content (entry), ==, "Test <markup> & escaping.");
	g_assert (gdata_entry_get_content_uri (entry) == NULL);
	g_assert_cmpstr (gdata_entry_get_rights (entry), ==, "Philip Withnall <philip@tecnocode.co.uk>");

	/* Set the properties more conventionally */
	gdata_entry_set_title (entry, "Testing title & escaping");
	gdata_entry_set_summary (entry, NULL);
	gdata_entry_set_content (entry, "This is some sample content testing, amongst other things, <markup> & odd characters‽");
	gdata_entry_set_rights (entry, NULL);

	/* Content URI */
	g_object_set (G_OBJECT (entry), "content-uri", "http://foo.com/", NULL);

	g_assert (gdata_entry_get_content (entry) == NULL);
	g_assert_cmpstr (gdata_entry_get_content_uri (entry), ==, "http://foo.com/");

	gdata_entry_set_content_uri (entry, "http://bar.com/");

	/* Check the generated XML's OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (entry));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'>"
				 "<title type='text'>Testing title &amp; escaping</title>"
				 "<content type='text/plain' src='http://bar.com/'/>"
			 "</entry>");

	/* Reset the content */
	gdata_entry_set_content (entry, "This is some sample content testing, amongst other things, <markup> & odd characters‽");

	/* Categories */
	category = gdata_category_new ("test", NULL, NULL);
	gdata_entry_add_category (entry, category);
	g_object_unref (category);
	category = gdata_category_new ("example", NULL, "Example stuff");
	gdata_entry_add_category (entry, category);
	g_object_unref (category);
	category = gdata_category_new ("Film", "http://gdata.youtube.com/schemas/2007/categories.cat", "Film & Animation");
	gdata_entry_add_category (entry, category);
	g_object_unref (category);

	/* Links */
	_link = gdata_link_new ("http://test.com/", GDATA_LINK_SELF);
	gdata_link_set_content_type (_link, "application/atom+xml");
	gdata_entry_add_link (entry, _link);
	g_object_unref (_link);
	_link = gdata_link_new ("http://example.com/", NULL);
	gdata_entry_add_link (entry, _link);
	g_object_unref (_link);
	_link = gdata_link_new ("http://test.mn/", GDATA_LINK_RELATED);
	gdata_link_set_content_type (_link, "text/html");
	gdata_link_set_language (_link, "mn");
	gdata_link_set_title (_link, "A treatise on Mongolian test websites & other stuff.");
	gdata_link_set_length (_link, 5010);
	gdata_entry_add_link (entry, _link);
	g_object_unref (_link);
	_link = gdata_link_new ("http://example.com/", "http://foobar.link");
	gdata_entry_add_link (entry, _link);
	g_object_unref (_link);
	_link = gdata_link_new ("http://example2.com/", "http://foobar.link");
	gdata_entry_add_link (entry, _link);
	g_object_unref (_link);

	/* Authors */
	author = gdata_author_new ("Joe Bloggs", "http://example.com/", "joe@example.com");
	gdata_entry_add_author (entry, author);
	g_object_unref (author);
	author = gdata_author_new ("John Smith", NULL, "smith.john@example.com");
	gdata_entry_add_author (entry, author);
	g_object_unref (author);
	author = gdata_author_new ("F. Barr؟", NULL, NULL);
	gdata_entry_add_author (entry, author);
	g_object_unref (author);

	/* Check the generated XML's OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (entry));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'>"
				 "<title type='text'>Testing title &amp; escaping</title>"
				 "<content type='text'>This is some sample content testing, amongst other things, &lt;markup&gt; &amp; odd characters\342\200\275</content>"
				 "<category term='Film' scheme='http://gdata.youtube.com/schemas/2007/categories.cat' label='Film &amp; Animation'/>"
				 "<category term='example' label='Example stuff'/>"
				 "<category term='test'/>"
				 "<link href='http://example2.com/' rel='http://foobar.link'/>"
				 "<link href='http://example.com/' rel='http://foobar.link'/>"
				 "<link href='http://test.mn/' title='A treatise on Mongolian test websites &amp; other stuff.' rel='http://www.iana.org/assignments/relation/related' type='text/html' hreflang='mn' length='5010'/>"
				 "<link href='http://example.com/' rel='http://www.iana.org/assignments/relation/alternate'/>"
				 "<link href='http://test.com/' rel='http://www.iana.org/assignments/relation/self' type='application/atom+xml'/>"
				 "<author><name>F. Barr\330\237</name></author>"
				 "<author><name>John Smith</name><email>smith.john@example.com</email></author>"
				 "<author><name>Joe Bloggs</name><uri>http://example.com/</uri><email>joe@example.com</email></author>"
			 "</entry>");

	/* Check again by re-parsing the XML to a GDataEntry */
	entry2 = GDATA_ENTRY (gdata_parsable_new_from_xml (GDATA_TYPE_ENTRY, xml, -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_ENTRY (entry2));
	g_clear_error (&error);
	g_free (xml);

	g_assert_cmpstr (gdata_entry_get_title (entry), ==, gdata_entry_get_title (entry2));
	g_assert_cmpstr (gdata_entry_get_id (entry), ==, gdata_entry_get_id (entry2)); /* should both be NULL */
	g_assert_cmpstr (gdata_entry_get_content (entry), ==, gdata_entry_get_content (entry2));
	g_assert_cmpstr (gdata_entry_get_content_uri (entry), ==, gdata_entry_get_content_uri (entry2)); /* should both be NULL */

	gdata_entry_get_updated (entry, &updated);
	gdata_entry_get_updated (entry2, &updated2);
	g_assert_cmpuint (updated.tv_sec, ==, updated2.tv_sec);
	g_assert_cmpuint (updated.tv_usec, ==, updated2.tv_usec);

	gdata_entry_get_published (entry, &published);
	gdata_entry_get_published (entry2, &published2);
	g_assert_cmpuint (published.tv_sec, ==, published2.tv_sec);
	g_assert_cmpuint (published.tv_usec, ==, published2.tv_usec);

	/* Check links */
	_link = gdata_entry_look_up_link (entry, GDATA_LINK_SELF);
	g_assert (_link != NULL);
	g_assert_cmpstr (gdata_link_get_uri (_link), ==, "http://test.com/");
	g_assert_cmpstr (gdata_link_get_relation_type (_link), ==, GDATA_LINK_SELF);
	g_assert_cmpstr (gdata_link_get_content_type (_link), ==, "application/atom+xml");

	_link = gdata_entry_look_up_link (entry, "http://link.not.exist");
	g_assert (_link == NULL);

	list = gdata_entry_look_up_links (entry, "http://foobar.link");
	g_assert (list != NULL);
	g_assert_cmpint (g_list_length (list), ==, 2);

	_link = GDATA_LINK (list->data);
	g_assert (_link != NULL);
	g_assert_cmpstr (gdata_link_get_uri (_link), ==, "http://example2.com/");
	g_assert_cmpstr (gdata_link_get_relation_type (_link), ==, "http://foobar.link");

	_link = GDATA_LINK (list->next->data);
	g_assert (_link != NULL);
	g_assert_cmpstr (gdata_link_get_uri (_link), ==, "http://example.com/");
	g_assert_cmpstr (gdata_link_get_relation_type (_link), ==, "http://foobar.link");

	g_list_free (list);

	/* Check authors */
	list = gdata_entry_get_authors (entry);
	g_assert_cmpuint (g_list_length (list), ==, 3);

	author = GDATA_AUTHOR (list->data);
	g_assert (author != NULL);
	author = gdata_author_new ("F. Barr؟", NULL, NULL);
	g_assert_cmpstr (gdata_author_get_name (author), ==, "F. Barr؟");
	g_assert (gdata_author_get_uri (author) == NULL);
	g_assert (gdata_author_get_email_address (author) == NULL);

	author = GDATA_AUTHOR (list->next->data);
	g_assert (author != NULL);
	g_assert_cmpstr (gdata_author_get_name (author), ==, "John Smith");
	g_assert (gdata_author_get_uri (author) == NULL);
	g_assert_cmpstr (gdata_author_get_email_address (author), ==, "smith.john@example.com");

	author = GDATA_AUTHOR (list->next->next->data);
	g_assert (author != NULL);
	g_assert_cmpstr (gdata_author_get_name (author), ==, "Joe Bloggs");
	g_assert_cmpstr (gdata_author_get_uri (author), ==, "http://example.com/");
	g_assert_cmpstr (gdata_author_get_email_address (author), ==, "joe@example.com");

	/* Check categories */
	list = gdata_entry_get_categories (entry);
	g_assert (list != NULL);
	g_assert_cmpint (g_list_length (list), ==, 3);

	category = GDATA_CATEGORY (list->data);
	g_assert (category != NULL);
	g_assert_cmpstr (gdata_category_get_term (category), ==, "Film");
	g_assert_cmpstr (gdata_category_get_scheme (category), ==, "http://gdata.youtube.com/schemas/2007/categories.cat");
	g_assert_cmpstr (gdata_category_get_label (category), ==, "Film & Animation");

	category = GDATA_CATEGORY (list->next->data);
	g_assert (category != NULL);
	g_assert_cmpstr (gdata_category_get_term (category), ==, "example");
	g_assert (gdata_category_get_scheme (category) == NULL);
	g_assert_cmpstr (gdata_category_get_label (category), ==, "Example stuff");

	category = GDATA_CATEGORY (list->next->next->data);
	g_assert (category != NULL);
	g_assert_cmpstr (gdata_category_get_term (category), ==, "test");
	g_assert (gdata_category_get_scheme (category) == NULL);
	g_assert (gdata_category_get_label (category) == NULL);

	/* TODO: Check authors */

	/* Check properties a different way */
	g_object_get (G_OBJECT (entry2),
	              "title", &title,
	              "summary", &summary,
	              "id", &id,
	              "etag", &etag,
	              "updated", &updated3,
	              "published", &published3,
	              "content", &content,
	              "content-uri", &content_uri,
	              "is-inserted", &is_inserted,
	              "rights", &rights,
	              NULL);

	g_assert_cmpstr (title, ==, gdata_entry_get_title (entry));
	g_assert_cmpstr (summary, ==, gdata_entry_get_summary (entry));
	g_assert_cmpstr (id, ==, gdata_entry_get_id (entry));
	g_assert_cmpstr (etag, ==, gdata_entry_get_etag (entry));
	g_assert_cmpint (updated3->tv_sec, ==, updated.tv_sec);
	g_assert_cmpint (updated3->tv_usec, ==, updated.tv_usec);
	g_assert_cmpint (published3->tv_sec, ==, published.tv_sec);
	g_assert_cmpint (published3->tv_usec, ==, published.tv_usec);
	g_assert_cmpstr (content, ==, gdata_entry_get_content (entry));
	g_assert_cmpstr (content_uri, ==, gdata_entry_get_content_uri (entry));
	g_assert (is_inserted == FALSE);
	g_assert_cmpstr (rights, ==, gdata_entry_get_rights (entry));

	g_free (title);
	g_free (summary);
	g_free (id);
	g_free (etag);
	g_free (updated3);
	g_free (published3);
	g_free (content);
	g_free (content_uri);
	g_free (rights);

	/* Set the content URI and check that */
	gdata_entry_set_content_uri (entry, "http://baz.net/");

	g_object_get (G_OBJECT (entry),
	              "content", &content,
	              "content-uri", &content_uri,
	              NULL);

	g_assert_cmpstr (content, ==, gdata_entry_get_content (entry));
	g_assert_cmpstr (content_uri, ==, gdata_entry_get_content_uri (entry));

	g_free (content);
	g_free (content_uri);

	g_object_unref (entry);
	g_object_unref (entry2);
}

static void
test_entry_parse_xml (void)
{
	GDataEntry *entry;
	gchar *xml;
	GError *error = NULL;

	/* Create an entry from XML with unhandled elements */
	entry = GDATA_ENTRY (gdata_parsable_new_from_xml (GDATA_TYPE_ENTRY,
		"<entry xmlns='http://www.w3.org/2005/Atom' xmlns:ns='http://example.com/'>"
			"<title type='text'>Testing unhandled XML</title>"
			"<updated>2009-01-25T14:07:37.880860Z</updated>"
			"<published>2009-01-23T14:06:37.880860Z</published>"
			"<content type='text'>Here we test unhandled XML elements.</content>"
			"<foobar>Test!</foobar>"
			"<barfoo shizzle='zing'/>"
			"<ns:barfoo shizzle='zing' fo='shizzle'>How about some characters‽</ns:barfoo>"
		 "</entry>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_ENTRY (entry));
	g_clear_error (&error);

	/* Now check the outputted XML from the entry still has the unhandled elements */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (entry));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' xmlns:ns='http://example.com/'>"
				"<title type='text'>Testing unhandled XML</title>"
				"<updated>2009-01-25T14:07:37.880860Z</updated>"
				"<published>2009-01-23T14:06:37.880860Z</published>"
				"<content type='text'>Here we test unhandled XML elements.</content>"
				"<foobar>Test!</foobar>"
				"<barfoo shizzle=\"zing\"/>"
				"<ns:barfoo shizzle=\"zing\" fo=\"shizzle\">How about some characters‽</ns:barfoo>"
			 "</entry>");
	g_free (xml);
}

static void
test_entry_error_handling (void)
{
	GDataEntry *entry;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) entry = GDATA_ENTRY (gdata_parsable_new_from_xml (GDATA_TYPE_ENTRY,\
		"<entry>"\
			x\
		"</entry>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (entry == NULL);\
	g_clear_error (&error)

	/* updated */
	TEST_XML_ERROR_HANDLING ("<updated>not a date</updated>"); /* invalid date */

	/* published */
	TEST_XML_ERROR_HANDLING ("<published>also not a date</published>"); /* invalid date */

	/* category */
	TEST_XML_ERROR_HANDLING ("<category/>"); /* invalid category */

	/* link */
	TEST_XML_ERROR_HANDLING ("<link/>"); /* invalid link */

	/* author */
	TEST_XML_ERROR_HANDLING ("<author/>"); /* invalid author */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_feed_parse_xml (void)
{
	GDataFeed *feed;
	GDataEntry *entry;
	GDataLink *_link; /* stupid unistd.h */
	GList *list;
	gchar *title, *subtitle, *id, *etag, *logo, *icon, *rights;
	GTimeVal *updated, updated2;
	GDataGenerator *generator;
	guint items_per_page, start_index, total_results;
	GError *error = NULL;

	feed = GDATA_FEED (gdata_parsable_new_from_xml (GDATA_TYPE_FEED,
		"<feed xmlns='http://www.w3.org/2005/Atom' "
		      "xmlns:openSearch='http://a9.com/-/spec/opensearch/1.1/' "
		      "xmlns:gd='http://schemas.google.com/g/2005' "
		      "gd:etag='W/\"D08FQn8-eil7ImA9WxZbFEw.\"'>"
			"<id>http://example.com/id</id>"
			"<updated>2009-02-25T14:07:37.880860Z</updated>"
			"<title type='text'>Test feed</title>"
			"<subtitle type='text'>Test subtitle</subtitle>"
			"<logo>http://example.com/logo.png</logo>"
			"<icon>http://example.com/icon.png</icon>"
			"<link rel='alternate' type='text/html' href='http://alternate.example.com/'/>"
			"<link rel='http://schemas.google.com/g/2005#feed' type='application/atom+xml' href='http://example.com/id'/>"
			"<link rel='http://schemas.google.com/g/2005#post' type='application/atom+xml' href='http://example.com/post'/>"
			"<link rel='self' type='application/atom+xml' href='http://example.com/id'/>"
			"<category scheme='http://example.com/categories' term='feed'/>"
			"<rights>public</rights>"
			"<author>"
				"<name>Joe Smith</name>"
				"<email>j.smith@example.com</email>"
			"</author>"
			"<generator version='0.6' uri='http://example.com/'>Example Generator</generator>"
			"<openSearch:totalResults>2</openSearch:totalResults>"
			"<openSearch:startIndex>1</openSearch:startIndex>"
			"<openSearch:itemsPerPage>50</openSearch:itemsPerPage>"
			"<entry>"
				"<id>entry1</id>"
				"<title type='text'>Testing unhandled XML</title>"
				"<updated>2009-01-25T14:07:37.880860Z</updated>"
				"<published>2009-01-23T14:06:37.880860Z</published>"
				"<content type='text'>Here we test unhandled XML elements.</content>"
			"</entry>"
			"<entry>"
				"<id>entry2</id>"
				"<title type='text'>Testing unhandled XML 2</title>"
				"<updated>2009-02-25T14:07:37.880860Z</updated>"
				"<published>2009-02-23T14:06:37.880860Z</published>"
				"<content type='text'>Here we test unhandled XML elements again.</content>"
			"</entry>"
			"<foobar>Test unhandled elements!</foobar>"
		"</feed>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_FEED (feed));
	g_clear_error (&error);

	/* Check the feed's properties */
	g_object_get (G_OBJECT (feed),
	              "title", &title,
	              "subtitle", &subtitle,
	              "id", &id,
	              "etag", &etag,
	              "updated", &updated,
	              "logo", &logo,
	              "icon", &icon,
	              "generator", &generator,
	              "rights", &rights,
	              "items-per-page", &items_per_page,
	              "start-index", &start_index,
	              "total-results", &total_results,
	              NULL);

	g_assert_cmpstr (title, ==, "Test feed");
	g_assert_cmpstr (subtitle, ==, "Test subtitle");
	g_assert_cmpstr (id, ==, "http://example.com/id");
	g_assert_cmpstr (etag, ==, "W/\"D08FQn8-eil7ImA9WxZbFEw.\"");

	g_assert_cmpint (updated->tv_sec, ==, 1235570857);
	g_assert_cmpint (updated->tv_usec, ==, 880860);

	g_assert_cmpstr (logo, ==, "http://example.com/logo.png");
	g_assert_cmpstr (icon, ==, "http://example.com/icon.png");
	g_assert_cmpstr (rights, ==, "public");

	g_assert (GDATA_IS_GENERATOR (generator));
	g_assert_cmpstr (gdata_generator_get_name (generator), ==, "Example Generator");
	g_assert_cmpstr (gdata_generator_get_uri (generator), ==, "http://example.com/");
	g_assert_cmpstr (gdata_generator_get_version (generator), ==, "0.6");

	g_assert_cmpuint (items_per_page, ==, 50);
	g_assert_cmpuint (start_index, ==, 1);
	g_assert_cmpuint (total_results, ==, 2);

	g_free (title);
	g_free (subtitle);
	g_free (id);
	g_free (etag);
	g_free (updated);
	g_free (logo);
	g_free (icon);
	g_free (rights);
	g_object_unref (generator);

	/* Check the entries */
	entry = gdata_feed_look_up_entry (feed, "entry1");
	g_assert (GDATA_IS_ENTRY (entry));

	entry = gdata_feed_look_up_entry (feed, "this doesn't exist");
	g_assert (entry == NULL);

	entry = gdata_feed_look_up_entry (feed, "entry2");
	g_assert (GDATA_IS_ENTRY (entry));

	/* Check the categories */
	list = gdata_feed_get_categories (feed);
	g_assert (list != NULL);
	g_assert_cmpint (g_list_length (list), ==, 1);
	g_assert (GDATA_IS_CATEGORY (list->data));

	/* Check the links */
	list = gdata_feed_get_links (feed);
	g_assert (list != NULL);
	g_assert_cmpint (g_list_length (list), ==, 4);
	g_assert (GDATA_IS_LINK (list->data));
	g_assert (GDATA_IS_LINK (list->next->data));
	g_assert (GDATA_IS_LINK (list->next->next->data));
	g_assert (GDATA_IS_LINK (list->next->next->next->data));

	_link = gdata_feed_look_up_link (feed, GDATA_LINK_ALTERNATE);
	g_assert (GDATA_IS_LINK (_link));

	_link = gdata_feed_look_up_link (feed, "this doesn't exist");
	g_assert (_link == NULL);

	_link = gdata_feed_look_up_link (feed, "http://schemas.google.com/g/2005#feed");
	g_assert (GDATA_IS_LINK (_link));

	_link = gdata_feed_look_up_link (feed, "http://schemas.google.com/g/2005#post");
	g_assert (GDATA_IS_LINK (_link));

	_link = gdata_feed_look_up_link (feed, GDATA_LINK_SELF);
	g_assert (GDATA_IS_LINK (_link));

	/* Check the authors */
	list = gdata_feed_get_authors (feed);
	g_assert (list != NULL);
	g_assert_cmpint (g_list_length (list), ==, 1);
	g_assert (GDATA_IS_AUTHOR (list->data));

	/* Check the other properties again, the more normal way this time */
	g_assert_cmpstr (gdata_feed_get_title (feed), ==, "Test feed");
	g_assert_cmpstr (gdata_feed_get_subtitle (feed), ==, "Test subtitle");
	g_assert_cmpstr (gdata_feed_get_id (feed), ==, "http://example.com/id");
	g_assert_cmpstr (gdata_feed_get_etag (feed), ==, "W/\"D08FQn8-eil7ImA9WxZbFEw.\"");

	gdata_feed_get_updated (feed, &updated2);
	g_assert_cmpint (updated2.tv_sec, ==, 1235570857);
	g_assert_cmpint (updated2.tv_usec, ==, 880860);

	g_assert_cmpstr (gdata_feed_get_logo (feed), ==, "http://example.com/logo.png");
	g_assert_cmpstr (gdata_feed_get_icon (feed), ==, "http://example.com/icon.png");
	g_assert_cmpstr (gdata_feed_get_rights (feed), ==, "public");

	generator = gdata_feed_get_generator (feed);
	g_assert (GDATA_IS_GENERATOR (generator));
	g_assert_cmpstr (gdata_generator_get_name (generator), ==, "Example Generator");
	g_assert_cmpstr (gdata_generator_get_uri (generator), ==, "http://example.com/");
	g_assert_cmpstr (gdata_generator_get_version (generator), ==, "0.6");

	g_assert_cmpuint (gdata_feed_get_items_per_page (feed), ==, 50);
	g_assert_cmpuint (gdata_feed_get_start_index (feed), ==, 1);
	g_assert_cmpuint (gdata_feed_get_total_results (feed), ==, 2);

	g_object_unref (feed);
}

static void
test_feed_error_handling (void)
{
	GDataFeed *feed;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) feed = GDATA_FEED (gdata_parsable_new_from_xml (GDATA_TYPE_FEED, \
		"<feed xmlns='http://www.w3.org/2005/Atom' xmlns:openSearch='http://a9.com/-/spec/opensearch/1.1/'>"\
			x\
		"</feed>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (feed == NULL);\
	g_clear_error (&error)

	/* entry */
	TEST_XML_ERROR_HANDLING ("<entry><updated>this isn't a date</updated></entry>"); /* invalid entry */

	/* title */
	TEST_XML_ERROR_HANDLING ("<title>First title</title><title>Second title</title>"); /* duplicate title */
	TEST_XML_ERROR_HANDLING ("<id>ID</id><updated>2009-01-25T14:07:37.880860Z</updated>"); /* missing title */

	/* subtitle */
	TEST_XML_ERROR_HANDLING ("<subtitle>First subtitle</subtitle><subtitle>Second subtitle</subtitle>"); /* duplicate subtitle */

	/* id */
	TEST_XML_ERROR_HANDLING ("<id>First ID</id><id>Second ID</id>"); /* duplicate ID */
	TEST_XML_ERROR_HANDLING ("<title>Title</title><updated>2009-01-25T14:07:37.880860Z</updated>"); /* missing ID */

	/* updated */
	TEST_XML_ERROR_HANDLING ("<updated>2009-01-25T14:07:37.880860Z</updated>"
	                         "<updated>2009-01-25T14:07:37.880860Z</updated>"); /* duplicate updated */
	TEST_XML_ERROR_HANDLING ("<updated>not a date</updated>"); /* invalid date */
	TEST_XML_ERROR_HANDLING ("<title>Title</title><id>ID</id>"); /* missing updated */

	/* category */
	TEST_XML_ERROR_HANDLING ("<category/>"); /* invalid category */

	/* logo */
	TEST_XML_ERROR_HANDLING ("<logo>First logo</logo><logo>Second logo</logo>"); /* duplicate logo */

	/* icon */
	TEST_XML_ERROR_HANDLING ("<icon>First icon</icon><icon>Second icon</icon>"); /* duplicate icon */

	/* link */
	TEST_XML_ERROR_HANDLING ("<link/>"); /* invalid link */

	/* author */
	TEST_XML_ERROR_HANDLING ("<author/>"); /* invalid author */

	/* generator */
	TEST_XML_ERROR_HANDLING ("<generator>First generator</generator><generator>Second generator</generator>"); /* duplicate generator */
	TEST_XML_ERROR_HANDLING ("<generator uri=''/>"); /* invalid generator */

	/* openSearch:totalResults */
	TEST_XML_ERROR_HANDLING ("<openSearch:totalResults>5</openSearch:totalResults>"
	                         "<openSearch:totalResults>3</openSearch:totalResults>"); /* duplicate totalResults */
	TEST_XML_ERROR_HANDLING ("<openSearch:totalResults></openSearch:totalResults>"); /* missing content */
	TEST_XML_ERROR_HANDLING ("<openSearch:totalResults>this isn't a number!</openSearch:totalResults>"); /* invalid number */

	/* openSearch:startIndex */
	TEST_XML_ERROR_HANDLING ("<openSearch:startIndex>5</openSearch:startIndex>"
	                         "<openSearch:startIndex>3</openSearch:startIndex>"); /* duplicate startIndex */
	TEST_XML_ERROR_HANDLING ("<openSearch:startIndex></openSearch:startIndex>"); /* missing content */
	TEST_XML_ERROR_HANDLING ("<openSearch:startIndex>this isn't a number!</openSearch:startIndex>"); /* invalid number */

	/* openSearch:itemsPerPage */
	TEST_XML_ERROR_HANDLING ("<openSearch:itemsPerPage>5</openSearch:itemsPerPage>"
	                         "<openSearch:itemsPerPage>3</openSearch:itemsPerPage>"); /* duplicate itemsPerPage */
	TEST_XML_ERROR_HANDLING ("<openSearch:itemsPerPage></openSearch:itemsPerPage>"); /* missing content */
	TEST_XML_ERROR_HANDLING ("<openSearch:itemsPerPage>this isn't a number!</openSearch:itemsPerPage>"); /* invalid number */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_query_categories (void)
{
	GDataQuery *query;
	gchar *query_uri;

	query = gdata_query_new ("foobar");

	/* AND */
	gdata_query_set_categories (query, "Fritz/Laurie");
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com/-/Fritz/Laurie?q=foobar");
	g_free (query_uri);

	/* OR */
	gdata_query_set_categories (query, "Fritz|Laurie");
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com/-/Fritz%7CLaurie?q=foobar");
	g_free (query_uri);

	/* Combination */
	gdata_query_set_categories (query, "A|-{urn:google.com}B/-C");
	query_uri = gdata_query_get_query_uri (query, "http://example.com/gdata_test");
	g_assert_cmpstr (query_uri, ==, "http://example.com/gdata_test/-/A%7C-%7Burn%3Agoogle.com%7DB/-C?q=foobar");
	g_free (query_uri);

	/* Same combination without q param */
	gdata_query_set_q (query, NULL);
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com/-/A%7C-%7Burn%3Agoogle.com%7DB/-C");
	g_free (query_uri);

	g_object_unref (query);
}

static void
test_query_unicode (void)
{
	GDataQuery *query;
	gchar *query_uri;

	g_test_bug ("602497");

	/* Simple query */
	query = gdata_query_new ("fööbar‽");
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com?q=f%C3%B6%C3%B6bar%E2%80%BD");
	g_free (query_uri);

	/* Categories */
	gdata_query_set_categories (query, "Ümlauts|¿Questions‽");
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com/-/%C3%9Cmlauts%7C%C2%BFQuestions%E2%80%BD?q=f%C3%B6%C3%B6bar%E2%80%BD");
	g_free (query_uri);

	/* Author */
	gdata_query_set_author (query, "Lørd Brïan Bleßêd");
	query_uri = gdata_query_get_query_uri (query, "http://example.com");
	g_assert_cmpstr (query_uri, ==, "http://example.com/-/%C3%9Cmlauts%7C%C2%BFQuestions%E2%80%BD?q=f%C3%B6%C3%B6bar%E2%80%BD&author=L%C3%B8rd%20Br%C3%AFan%20Ble%C3%9F%C3%AAd");
	g_free (query_uri);

	g_object_unref (query);
}

static void
test_query_etag (void)
{
	GDataQuery *query = gdata_query_new (NULL);

	/* Test that setting any property will unset the ETag */
	g_test_bug ("613529");

	/* Also check that setting the ETag doesn't unset the ETag! */
	gdata_query_set_etag (query, "foobar");
	g_assert_cmpstr (gdata_query_get_etag (query), ==, "foobar");

#define CHECK_ETAG(C) \
	gdata_query_set_etag (query, "foobar");		\
	(C);						\
	g_assert (gdata_query_get_etag (query) == NULL);

	CHECK_ETAG (gdata_query_set_q (query, "q"))
	CHECK_ETAG (gdata_query_set_categories (query, "shizzle,foobar"))
	CHECK_ETAG (gdata_query_set_author (query, "John Smith"))
	CHECK_ETAG (gdata_query_set_updated_min (query, NULL))
	CHECK_ETAG (gdata_query_set_updated_max (query, NULL))
	CHECK_ETAG (gdata_query_set_published_min (query, NULL))
	CHECK_ETAG (gdata_query_set_published_max (query, NULL))
	CHECK_ETAG (gdata_query_set_start_index (query, 5))
	CHECK_ETAG (gdata_query_set_is_strict (query, TRUE))
	CHECK_ETAG (gdata_query_set_max_results (query, 1000))
	CHECK_ETAG (gdata_query_next_page (query))
	CHECK_ETAG (gdata_query_previous_page (query))

#undef CHECK_ETAG

	g_object_unref (query);
}

static void
test_service_network_error (void)
{
	GDataService *service;
	SoupURI *proxy_uri;
	GError *error = NULL;

	/* This is a little hacky, but it should work */
	service = g_object_new (GDATA_TYPE_SERVICE, "client-id", CLIENT_ID, NULL);

	/* Try a query which should always fail due to errors resolving the hostname */
	g_assert (gdata_service_query (service, "http://thisshouldnotexist.localhost", NULL, GDATA_TYPE_ENTRY, NULL, NULL, NULL, &error) == NULL);
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_NETWORK_ERROR);
	g_clear_error (&error);

	/* Try one with a bad proxy set */
	proxy_uri = soup_uri_new ("http://thisshouldalsonotexist.localhost/proxy");
	gdata_service_set_proxy_uri (service, proxy_uri);
	soup_uri_free (proxy_uri);

	g_assert (gdata_service_query (service, "http://google.com", NULL, GDATA_TYPE_ENTRY, NULL, NULL, NULL, &error) == NULL);
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROXY_ERROR);
	g_clear_error (&error);

	g_object_unref (service);
}

static void
test_service_locale (void)
{
	GDataService *service;
	gchar *locale;

	/* This is a little hacky, but it should work */
	service = g_object_new (GDATA_TYPE_SERVICE, "client-id", CLIENT_ID, NULL);

	/* Just test setting and getting the locale */
	g_assert (gdata_service_get_locale (service) == NULL);
	gdata_service_set_locale (service, "en_GB");
	g_assert_cmpstr (gdata_service_get_locale (service), ==, "en_GB");

	g_object_get (service, "locale", &locale, NULL);
	g_assert_cmpstr (locale, ==, "en_GB");
	g_free (locale);

	g_object_unref (service);
}

static void
test_access_rule_get_xml (void)
{
	GDataAccessRule *rule, *rule2;
	gchar *xml, *role, *scope_type3, *scope_value3;
	GTimeVal edited, *edited2;
	const gchar *scope_type, *scope_value, *scope_type2, *scope_value2;
	GError *error = NULL;

	rule = gdata_access_rule_new ("an-id");

	/* Test setting properties directly */
	g_object_set (G_OBJECT (rule),
	              "role", "A role",
	              "scope-type", "A scope type",
	              "scope-value", "A scope value",
	              NULL);

	g_assert_cmpstr (gdata_access_rule_get_role (rule), ==, "A role");
	gdata_access_rule_get_scope (rule, &scope_type, &scope_value);
	g_assert_cmpstr (scope_type, ==, "A scope type");
	g_assert_cmpstr (scope_value, ==, "A scope value");
	gdata_access_rule_get_edited (rule, &edited);
	g_assert_cmpuint (edited.tv_sec, >, 0); /* current time */

	/* Set the properties more conventionally */
	gdata_access_rule_set_role (rule, GDATA_ACCESS_ROLE_NONE);
	gdata_access_rule_set_scope (rule, GDATA_ACCESS_SCOPE_USER, "foo@example.com");

	/* Check the generated XML's OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (rule));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<entry xmlns='http://www.w3.org/2005/Atom' "
			 "xmlns:gd='http://schemas.google.com/g/2005' "
			 "xmlns:gAcl='http://schemas.google.com/acl/2007'>"
				"<title type='text'>none</title>"
				"<id>an-id</id>"
				"<category term='http://schemas.google.com/acl/2007#accessRule' scheme='http://schemas.google.com/g/2005#kind'/>"
				"<gAcl:role value='none'/>"
				"<gAcl:scope type='user' value='foo@example.com'/>"
			 "</entry>");

	/* Check again by re-parsing the XML to a GDataAccessRule */
	rule2 = GDATA_ACCESS_RULE (gdata_parsable_new_from_xml (GDATA_TYPE_ACCESS_RULE, xml, -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_ACCESS_RULE (rule2));
	g_clear_error (&error);
	g_free (xml);

	g_assert_cmpstr (gdata_access_rule_get_role (rule), ==, gdata_access_rule_get_role (rule2));
	gdata_access_rule_get_scope (rule, &scope_type, &scope_value);
	gdata_access_rule_get_scope (rule2, &scope_type2, &scope_value2);
	g_assert_cmpstr (scope_type, ==, scope_type2);
	g_assert_cmpstr (scope_value, ==, scope_value2);
	gdata_access_rule_get_edited (rule2, &edited);
	g_assert_cmpuint (edited.tv_sec, ==, 0); /* unspecified in XML */
	g_assert_cmpuint (edited.tv_usec, ==, 0);

	/* Check properties a different way */
	g_object_get (G_OBJECT (rule2),
	              "role", &role,
	              "scope-type", &scope_type3,
	              "scope-value", &scope_value3,
	              "edited", &edited2,
	              NULL);

	g_assert_cmpstr (role, ==, gdata_access_rule_get_role (rule));
	g_assert_cmpstr (scope_type, ==, scope_type3);
	g_assert_cmpstr (scope_value, ==, scope_value3);
	g_assert_cmpuint (edited2->tv_sec, ==, 0);
	g_assert_cmpuint (edited2->tv_usec, ==, 0);

	g_free (edited2);
	g_free (role);
	g_free (scope_type3);
	g_free (scope_value3);

	/* Test that the GDataAccessRule:role and GDataEntry:title properties are linked */
	gdata_entry_set_title (GDATA_ENTRY (rule), "Another role");
	g_assert_cmpstr (gdata_access_rule_get_role (rule), ==, "Another role");
	gdata_access_rule_set_role (rule, GDATA_ACCESS_ROLE_NONE);
	g_assert_cmpstr (gdata_entry_get_title (GDATA_ENTRY (rule)), ==, "none");

	g_object_unref (rule);
	g_object_unref (rule2);
}

static void
test_access_rule_error_handling (void)
{
	GDataAccessRule *rule;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) rule = GDATA_ACCESS_RULE (gdata_parsable_new_from_xml (GDATA_TYPE_ACCESS_RULE,\
		"<entry xmlns='http://www.w3.org/2005/Atom' xmlns:app='http://www.w3.org/2007/app' xmlns:gAcl='http://schemas.google.com/acl/2007'>"\
			x\
		"</entry>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (rule == NULL);\
	g_clear_error (&error)

	/* role */
	TEST_XML_ERROR_HANDLING ("<gAcl:role/>"); /* missing value */

	/* scope */
	TEST_XML_ERROR_HANDLING ("<gAcl:scope/>"); /* missing type */

	/* edited */
	TEST_XML_ERROR_HANDLING ("<app:edited/>"); /* missing date */
	TEST_XML_ERROR_HANDLING ("<app:edited>not a date</app:edited>"); /* bad date */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_comparable (void)
{
	GDataComparable *category = GDATA_COMPARABLE (gdata_category_new ("term", "http://scheme", "label"));

	/* Test the NULL comparisons */
	g_assert_cmpint (gdata_comparable_compare (category, NULL), ==, 1);
	g_assert_cmpint (gdata_comparable_compare (NULL, category), ==, -1);
	g_assert_cmpint (gdata_comparable_compare (NULL, NULL), ==, 0);
	g_assert_cmpint (gdata_comparable_compare (category, category), ==, 0);
}

static void
test_color_parsing (void)
{
	GDataColor color;

	/* With hash */
	g_assert (gdata_color_from_hexadecimal ("#F99Ff0", &color) == TRUE);
	g_assert_cmpuint (color.red, ==, 249);
	g_assert_cmpuint (color.green, ==, 159);
	g_assert_cmpuint (color.blue, ==, 240);

	/* Without hash */
	g_assert (gdata_color_from_hexadecimal ("F99Ff0", &color) == TRUE);
	g_assert_cmpuint (color.red, ==, 249);
	g_assert_cmpuint (color.green, ==, 159);
	g_assert_cmpuint (color.blue, ==, 240);

	/* Invalid, but correct length */
	g_assert (gdata_color_from_hexadecimal ("foobar", &color) == FALSE);

	/* Wildly invalid */
	g_assert (gdata_color_from_hexadecimal ("this is not a real colour!", &color) == FALSE);
}

static void
test_color_output (void)
{
	GDataColor color;
	gchar *color_string;

	/* General test */
	g_assert (gdata_color_from_hexadecimal ("#F99Ff0", &color) == TRUE);
	color_string = gdata_color_to_hexadecimal (&color);
	g_assert_cmpstr (color_string, ==, "#f99ff0");
	g_free (color_string);

	/* Boundary tests */
	g_assert (gdata_color_from_hexadecimal ("#ffffff", &color) == TRUE);
	color_string = gdata_color_to_hexadecimal (&color);
	g_assert_cmpstr (color_string, ==, "#ffffff");
	g_free (color_string);

	g_assert (gdata_color_from_hexadecimal ("#000000", &color) == TRUE);
	color_string = gdata_color_to_hexadecimal (&color);
	g_assert_cmpstr (color_string, ==, "#000000");
	g_free (color_string);
}

/*static void
test_media_thumbnail_parse_time (const gchar *locale)
{
	g_test_bug ("584737");

	g_test_message ("Testing gdata_media_thumbnail_parse_time in the \"%s\" locale...", locale);
	g_assert_cmpstr (setlocale (LC_ALL, locale), ==, locale);

	g_assert_cmpint (gdata_media_thumbnail_parse_time ("00:01:42.500"), ==, 102500);
	g_assert_cmpint (gdata_media_thumbnail_parse_time ("00:02:45"), ==, 165000);
	g_assert_cmpint (gdata_media_thumbnail_parse_time ("12:00:15.000"), ==, 43215000);
	g_assert_cmpint (gdata_media_thumbnail_parse_time ("00:00:00"), ==, 0);
	g_assert_cmpint (gdata_media_thumbnail_parse_time ("foobar"), ==, -1);

	setlocale (LC_ALL, "");
}*/

static void
test_atom_author (void)
{
	GDataAuthor *author, *author2;
	gchar *xml, *name, *uri, *email_address;
	GError *error = NULL;

	author = GDATA_AUTHOR (gdata_parsable_new_from_xml (GDATA_TYPE_AUTHOR,
		"<author>"
			"<name>John Smöth</name>"
			"<uri>http://example.com/</uri>"
			"<email>john@example.com</email>"
		"</author>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_AUTHOR (author));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_author_get_name (author), ==, "John Smöth");
	g_assert_cmpstr (gdata_author_get_uri (author), ==, "http://example.com/");
	g_assert_cmpstr (gdata_author_get_email_address (author), ==, "john@example.com");

	/* Compare it against another identical author */
	author2 = gdata_author_new ("John Smöth", "http://example.com/", "john@example.com");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (author), GDATA_COMPARABLE (author2)), ==, 0);
	g_object_unref (author2);

	/* …and a different author */
	author2 = gdata_author_new ("Brian Blessed", NULL, NULL);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (author), GDATA_COMPARABLE (author2)), !=, 0);
	g_object_unref (author2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (author));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<author xmlns='http://www.w3.org/2005/Atom'>"
				"<name>John Smöth</name>"
				"<uri>http://example.com/</uri>"
				"<email>john@example.com</email>"
			 "</author>");
	g_free (xml);

	/* Check the properties */
	g_object_get (G_OBJECT (author),
	              "name", &name,
	              "uri", &uri,
	              "email-address", &email_address,
	              NULL);

	g_assert_cmpstr (name, ==, "John Smöth");
	g_assert_cmpstr (uri, ==, "http://example.com/");
	g_assert_cmpstr (email_address, ==, "john@example.com");

	g_free (name);
	g_free (uri);
	g_free (email_address);
	g_object_unref (author);

	/* Now parse an author with little information available */
	author = GDATA_AUTHOR (gdata_parsable_new_from_xml (GDATA_TYPE_AUTHOR,
		"<author>"
			"<name>James Johnson</name>"
		"</author>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_AUTHOR (author));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_author_get_name (author), ==, "James Johnson");
	g_assert (gdata_author_get_uri (author) == NULL);
	g_assert (gdata_author_get_email_address (author) == NULL);

	g_object_unref (author);
}

static void
test_atom_author_error_handling (void)
{
	GDataAuthor *author;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) author = GDATA_AUTHOR (gdata_parsable_new_from_xml (GDATA_TYPE_AUTHOR,\
		"<author>"\
			x\
		"</author>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (author == NULL);\
	g_clear_error (&error)

	/* name */
	TEST_XML_ERROR_HANDLING ("<name>John Smöth</name><name>Not John Smöth</name>"); /* duplicated name */
	TEST_XML_ERROR_HANDLING ("<name></name>"); /* empty name */
	TEST_XML_ERROR_HANDLING ("<uri>http://example.com/</uri><email>john@example.com</email>"); /* missing name */

	/* uri */
	TEST_XML_ERROR_HANDLING ("<uri>http://example.com/</uri><uri>http://another-example.com/</uri>"); /* duplicated URI */

	/* email */
	TEST_XML_ERROR_HANDLING ("<email>john@example.com</email><email>john@another-example.com</email>"); /* duplicated e-mail address */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_atom_category (void)
{
	GDataCategory *category, *category2;
	gchar *xml, *term, *scheme, *label;
	GError *error = NULL;

	category = GDATA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_CATEGORY,
		"<category term='jokes' scheme='http://foobar.com#categories' label='Jokes &amp; Trivia'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_CATEGORY (category));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_category_get_term (category), ==, "jokes");
	g_assert_cmpstr (gdata_category_get_scheme (category), ==, "http://foobar.com#categories");
	g_assert_cmpstr (gdata_category_get_label (category), ==, "Jokes & Trivia");

	/* Compare it against another identical category */
	category2 = gdata_category_new ("jokes", "http://foobar.com#categories", "Jokes & Trivia");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (category), GDATA_COMPARABLE (category2)), ==, 0);
	g_object_unref (category2);

	/* …and a different category */
	category2 = gdata_category_new ("sports", "http://foobar.com#categories", NULL);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (category), GDATA_COMPARABLE (category2)), !=, 0);
	g_object_unref (category2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (category));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<category xmlns='http://www.w3.org/2005/Atom' "
				"term='jokes' scheme='http://foobar.com#categories' label='Jokes &amp; Trivia'/>");
	g_free (xml);

	/* Check the properties */
	g_object_get (G_OBJECT (category),
	              "term", &term,
	              "scheme", &scheme,
	              "label", &label,
	              NULL);

	g_assert_cmpstr (term, ==, "jokes");
	g_assert_cmpstr (scheme, ==, "http://foobar.com#categories");
	g_assert_cmpstr (label, ==, "Jokes & Trivia");

	g_free (term);
	g_free (scheme);
	g_free (label);
	g_object_unref (category);

	/* Now parse a category with less information available */
	category = GDATA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_CATEGORY,
		"<category term='sports'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_CATEGORY (category));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_category_get_term (category), ==, "sports");
	g_assert (gdata_category_get_scheme (category) == NULL);
	g_assert (gdata_category_get_label (category) == NULL);
	g_object_unref (category);

	/* Try a category with custom content */
	category = GDATA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_CATEGORY,
		"<category term='documentary'>"
			"<foobar/>"
			"<shizzle/>"
		"</category>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_CATEGORY (category));
	g_clear_error (&error);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (category));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<category xmlns='http://www.w3.org/2005/Atom' term='documentary'>"
				"<foobar/>"
				"<shizzle/>"
			 "</category>");
	g_free (xml);
	g_object_unref (category);
}

static void
test_atom_category_error_handling (void)
{
	GDataCategory *category;
	GError *error = NULL;

	/* Missing term */
	category = GDATA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_CATEGORY, "<category/>", -1, &error));
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);
	g_assert (category == NULL);
	g_clear_error (&error);
}

static void
test_atom_generator (void)
{
	GDataGenerator *generator, *generator2;
	gchar *name, *uri, *version;
	GError *error = NULL;

	generator = GDATA_GENERATOR (gdata_parsable_new_from_xml (GDATA_TYPE_GENERATOR,
		"<generator uri='http://example.com/' version='15'>Bach &amp; Son's Generator</generator>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GENERATOR (generator));
	g_clear_error (&error);

	/* Compare it against another identical generator */
	generator2 = GDATA_GENERATOR (gdata_parsable_new_from_xml (GDATA_TYPE_GENERATOR,
		"<generator uri='http://example.com/' version='15'>Bach &amp; Son's Generator</generator>", -1, NULL));
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (generator), GDATA_COMPARABLE (generator2)), ==, 0);
	g_object_unref (generator2);

	/* …and a different generator */
	generator2 = GDATA_GENERATOR (gdata_parsable_new_from_xml (GDATA_TYPE_GENERATOR,
		"<generator>Different generator</generator>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GENERATOR (generator));
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (generator), GDATA_COMPARABLE (generator2)), !=, 0);
	g_object_unref (generator2);

	/* Check the properties */
	g_assert_cmpstr (gdata_generator_get_name (generator), ==, "Bach & Son's Generator");
	g_assert_cmpstr (gdata_generator_get_uri (generator), ==, "http://example.com/");
	g_assert_cmpstr (gdata_generator_get_version (generator), ==, "15");

	/* Check them a different way too */
	g_object_get (G_OBJECT (generator),
	              "name", &name,
	              "uri", &uri,
	              "version", &version,
	              NULL);

	g_assert_cmpstr (name, ==, "Bach & Son's Generator");
	g_assert_cmpstr (uri, ==, "http://example.com/");
	g_assert_cmpstr (version, ==, "15");

	g_free (name);
	g_free (uri);
	g_free (version);
	g_object_unref (generator);

	/* Now parse a generator with less information available */
	generator = GDATA_GENERATOR (gdata_parsable_new_from_xml (GDATA_TYPE_GENERATOR,
		"<generator/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GENERATOR (generator));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_generator_get_name (generator) == NULL);
	g_assert (gdata_generator_get_uri (generator) == NULL);
	g_assert (gdata_generator_get_version (generator) == NULL);
	g_object_unref (generator);
}

static void
test_atom_generator_error_handling (void)
{
	GDataGenerator *generator;
	GError *error = NULL;

	/* Empty URI */
	generator = GDATA_GENERATOR (gdata_parsable_new_from_xml (GDATA_TYPE_GENERATOR, "<generator uri=''/>", -1, &error));
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);
	g_assert (generator == NULL);
	g_clear_error (&error);
}

static void
test_atom_link (void)
{
	GDataLink *link1, *link2;
	gchar *xml, *uri, *relation_type, *content_type, *language, *title;
	gint length;
	GError *error = NULL;

	link1 = GDATA_LINK (gdata_parsable_new_from_xml (GDATA_TYPE_LINK,
		"<link href='http://example.com/' rel='http://test.com#link-type' type='text/plain' hreflang='de' "
			"title='All About Angle Brackets: &lt;, &gt;' length='2000'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_LINK (link1));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_link_get_uri (link1), ==, "http://example.com/");
	g_assert_cmpstr (gdata_link_get_relation_type (link1), ==, "http://test.com#link-type");
	g_assert_cmpstr (gdata_link_get_content_type (link1), ==, "text/plain");
	g_assert_cmpstr (gdata_link_get_language (link1), ==, "de");
	g_assert_cmpstr (gdata_link_get_title (link1), ==, "All About Angle Brackets: <, >");
	g_assert_cmpint (gdata_link_get_length (link1), ==, 2000);

	/* Compare it against another identical link */
	link2 = gdata_link_new ("http://example.com/", "http://test.com#link-type");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (link1), GDATA_COMPARABLE (link2)), ==, 0);
	gdata_link_set_content_type (link2, "text/plain");
	gdata_link_set_language (link2, "de");
	gdata_link_set_title (link2, "All About Angle Brackets: <, >");
	gdata_link_set_length (link2, 2000);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (link1), GDATA_COMPARABLE (link2)), ==, 0);

	/* Try with a dissimilar link */
	gdata_link_set_uri (link2, "http://gnome.org/");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (link1), GDATA_COMPARABLE (link2)), !=, 0);
	g_object_unref (link2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (link1));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<link xmlns='http://www.w3.org/2005/Atom' href='http://example.com/' title='All About Angle Brackets: &lt;, &gt;' "
				"rel='http://test.com#link-type' type='text/plain' hreflang='de' length='2000'/>");
	g_free (xml);

	/* Set some of the properties */
	g_object_set (G_OBJECT (link1),
	              "uri", "http://another-example.com/",
	              "relation-type", "http://test.com#link-type2",
	              "content-type", "text/html",
	              "language", "sv",
	              "title", "This & That About <Angle Brackets>",
	              "length", -1,
	              NULL);

	/* Check the properties */
	g_object_get (G_OBJECT (link1),
	              "uri", &uri,
	              "relation-type", &relation_type,
	              "content-type", &content_type,
	              "language", &language,
	              "title", &title,
	              "length", &length,
	              NULL);

	g_assert_cmpstr (uri, ==, "http://another-example.com/");
	g_assert_cmpstr (relation_type, ==, "http://test.com#link-type2");
	g_assert_cmpstr (content_type, ==, "text/html");
	g_assert_cmpstr (language, ==, "sv");
	g_assert_cmpstr (title, ==, "This & That About <Angle Brackets>");
	g_assert_cmpint (length, ==, -1);

	g_free (uri);
	g_free (relation_type);
	g_free (content_type);
	g_free (language);
	g_free (title);
	g_object_unref (link1);

	/* Now parse a link with less information available */
	link1 = GDATA_LINK (gdata_parsable_new_from_xml (GDATA_TYPE_LINK,
		"<link href='http://shizzle.com'>Test Content<foobar/></link>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_LINK (link1));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_link_get_uri (link1), ==, "http://shizzle.com");
	g_assert_cmpstr (gdata_link_get_relation_type (link1), ==, "http://www.iana.org/assignments/relation/alternate");
	g_assert (gdata_link_get_content_type (link1) == NULL);
	g_assert (gdata_link_get_language (link1) == NULL);
	g_assert (gdata_link_get_title (link1) == NULL);
	g_assert (gdata_link_get_length (link1) == -1);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (link1));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<link xmlns='http://www.w3.org/2005/Atom' href='http://shizzle.com' rel='http://www.iana.org/assignments/relation/alternate'>"
				"Test Content<foobar/></link>");
	g_free (xml);
	g_object_unref (link1);
}

static void
test_atom_link_error_handling (void)
{
	GDataLink *_link; /* stupid unistd.h */
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) _link = GDATA_LINK (gdata_parsable_new_from_xml (GDATA_TYPE_LINK,\
		"<link " x "/>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (_link == NULL);\
	g_clear_error (&error)

	/* href */
	TEST_XML_ERROR_HANDLING (""); /* missing href */
	TEST_XML_ERROR_HANDLING ("href=''"); /* empty href */

	/* rel */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel=''"); /* empty rel */

	/* type */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' type=''"); /* empty type */

	/* hreflang */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' hreflang=''"); /* empty hreflang */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_app_categories (void)
{
	GDataAPPCategories *categories;
	GList *_categories;
	gboolean fixed;
	GError *error = NULL;

	categories = GDATA_APP_CATEGORIES (gdata_parsable_new_from_xml (GDATA_TYPE_APP_CATEGORIES,
		"<app:categories xmlns:app='http://www.w3.org/2007/app' fixed='yes' scheme='http://scheme'>"
			"<category term='foo'/>"
			"<category scheme='http://other.scheme' term='bar'/>"
		"</app:categories>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_APP_CATEGORIES (categories));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_app_categories_is_fixed (categories) == TRUE);

	/* Check them a different way too */
	g_object_get (G_OBJECT (categories), "is-fixed", &fixed, NULL);
	g_assert (fixed == TRUE);

	/* Check the categories and scheme inheritance */
	_categories = gdata_app_categories_get_categories (categories);
	g_assert_cmpint (g_list_length (_categories), ==, 2);

	g_assert (GDATA_IS_CATEGORY (_categories->data));
	g_assert_cmpstr (gdata_category_get_scheme (GDATA_CATEGORY (_categories->data)), ==, "http://scheme");

	g_assert (GDATA_IS_CATEGORY (_categories->next->data));
	g_assert_cmpstr (gdata_category_get_scheme (GDATA_CATEGORY (_categories->next->data)), ==, "http://other.scheme");

	g_object_unref (categories);

	/* Now parse one with less information available */
	categories = GDATA_APP_CATEGORIES (gdata_parsable_new_from_xml (GDATA_TYPE_APP_CATEGORIES,
		"<app:categories xmlns:app='http://www.w3.org/2007/app'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_APP_CATEGORIES (categories));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_app_categories_is_fixed (categories) == FALSE);
	g_object_unref (categories);
}

static void
test_gd_email_address (void)
{
	GDataGDEmailAddress *email, *email2;
	gchar *xml;
	GError *error = NULL;

	email = GDATA_GD_EMAIL_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_EMAIL_ADDRESS,
		"<gd:email xmlns:gd='http://schemas.google.com/g/2005' label='Personal &amp; Private' rel='http://schemas.google.com/g/2005#home' "
			"address='fubar@gmail.com' primary='true' displayName='&lt;John Smith&gt;'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_EMAIL_ADDRESS (email));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_email_address_get_address (email), ==, "fubar@gmail.com");
	g_assert_cmpstr (gdata_gd_email_address_get_relation_type (email), ==, GDATA_GD_EMAIL_ADDRESS_HOME);
	g_assert_cmpstr (gdata_gd_email_address_get_label (email), ==, "Personal & Private");
	g_assert_cmpstr (gdata_gd_email_address_get_display_name (email), ==, "<John Smith>");
	g_assert (gdata_gd_email_address_is_primary (email) == TRUE);

	/* Compare it against another identical address */
	email2 = gdata_gd_email_address_new ("fubar@gmail.com", GDATA_GD_EMAIL_ADDRESS_HOME, "Personal & Private", TRUE);
	gdata_gd_email_address_set_display_name (email2, "<John Smith>");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (email), GDATA_COMPARABLE (email2)), ==, 0);

	/* …and a different one */
	gdata_gd_email_address_set_address (email2, "test@example.com");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (email), GDATA_COMPARABLE (email2)), !=, 0);
	g_object_unref (email2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (email));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:email xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' address='fubar@gmail.com' "
				"rel='http://schemas.google.com/g/2005#home' label='Personal &amp; Private' displayName='&lt;John Smith&gt;' "
				"primary='true'/>");
	g_free (xml);
	g_object_unref (email);

	/* Now parse an address with less information available */
	email = GDATA_GD_EMAIL_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_EMAIL_ADDRESS,
		"<gd:email xmlns:gd='http://schemas.google.com/g/2005' address='test@example.com'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_EMAIL_ADDRESS (email));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_email_address_get_address (email), ==, "test@example.com");
	g_assert (gdata_gd_email_address_get_relation_type (email) == NULL);
	g_assert (gdata_gd_email_address_get_label (email) == NULL);
	g_assert (gdata_gd_email_address_get_display_name (email) == NULL);
	g_assert (gdata_gd_email_address_is_primary (email) == FALSE);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (email));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:email xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' address='test@example.com' "
				"primary='false'/>");
	g_free (xml);
	g_object_unref (email);
}

static void
test_gd_email_address_escaping (void)
{
	GDataGDEmailAddress *email;
	gchar *xml;

	g_test_bug ("630350");

	email = gdata_gd_email_address_new ("Fubar <fubar@gmail.com>", GDATA_GD_EMAIL_ADDRESS_HOME "?foo&bar", "Personal & Private", TRUE);
	gdata_gd_email_address_set_display_name (email, "<John Smith>");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (email));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:email xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                           "address='Fubar &lt;fubar@gmail.com&gt;' rel='http://schemas.google.com/g/2005#home?foo&amp;bar' "
	                           "label='Personal &amp; Private' displayName='&lt;John Smith&gt;' primary='true'/>");
	g_free (xml);
	g_object_unref (email);
}

static void
test_gd_im_address (void)
{
	GDataGDIMAddress *im, *im2;
	gchar *xml;
	GError *error = NULL;

	im = GDATA_GD_IM_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_IM_ADDRESS,
		"<gd:im xmlns:gd='http://schemas.google.com/g/2005' protocol='http://schemas.google.com/g/2005#MSN' address='foo@bar.msn.com' "
			"rel='http://schemas.google.com/g/2005#home' primary='true'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_IM_ADDRESS (im));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_im_address_get_address (im), ==, "foo@bar.msn.com");
	g_assert_cmpstr (gdata_gd_im_address_get_protocol (im), ==, GDATA_GD_IM_PROTOCOL_LIVE_MESSENGER);
	g_assert_cmpstr (gdata_gd_im_address_get_relation_type (im), ==, GDATA_GD_IM_ADDRESS_HOME);
	g_assert (gdata_gd_im_address_get_label (im) == NULL);
	g_assert (gdata_gd_im_address_is_primary (im) == TRUE);

	/* Compare it against another identical address */
	im2 = gdata_gd_im_address_new ("foo@bar.msn.com", GDATA_GD_IM_PROTOCOL_LIVE_MESSENGER, GDATA_GD_IM_ADDRESS_HOME, NULL, TRUE);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (im), GDATA_COMPARABLE (im2)), ==, 0);

	/* …and a different one */
	gdata_gd_im_address_set_protocol (im2, GDATA_GD_IM_PROTOCOL_GOOGLE_TALK);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (im), GDATA_COMPARABLE (im2)), !=, 0);
	g_object_unref (im2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (im));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:im xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"address='foo@bar.msn.com' protocol='http://schemas.google.com/g/2005#MSN' "
				"rel='http://schemas.google.com/g/2005#home' primary='true'/>");
	g_free (xml);
	g_object_unref (im);

	/* Now parse an address with less information available */
	im = GDATA_GD_IM_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_IM_ADDRESS,
		"<gd:im xmlns:gd='http://schemas.google.com/g/2005' label='Other &amp; Miscellaneous' address='foo@baz.example.com'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_IM_ADDRESS (im));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_im_address_get_address (im), ==, "foo@baz.example.com");
	g_assert (gdata_gd_im_address_get_protocol (im) == NULL);
	g_assert (gdata_gd_im_address_get_relation_type (im) == NULL);
	g_assert_cmpstr (gdata_gd_im_address_get_label (im), ==, "Other & Miscellaneous");
	g_assert (gdata_gd_im_address_is_primary (im) == FALSE);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (im));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:im xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' address='foo@baz.example.com' "
				"label='Other &amp; Miscellaneous' primary='false'/>");
	g_free (xml);
	g_object_unref (im);
}

static void
test_gd_im_address_escaping (void)
{
	GDataGDIMAddress *im;
	gchar *xml;

	im = gdata_gd_im_address_new ("Fubar <fubar@gmail.com>", GDATA_GD_IM_PROTOCOL_GOOGLE_TALK "?foo&bar", GDATA_GD_IM_ADDRESS_HOME "?foo&bar",
	                              "Personal & Private", TRUE);

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (im));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:im xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                        "address='Fubar &lt;fubar@gmail.com&gt;' protocol='http://schemas.google.com/g/2005#GOOGLE_TALK?foo&amp;bar' "
	                        "rel='http://schemas.google.com/g/2005#home?foo&amp;bar' label='Personal &amp; Private' primary='true'/>");
	g_free (xml);
	g_object_unref (im);
}

static void
test_gd_name (void)
{
	GDataGDName *name, *name2;
	gchar *xml;
	GError *error = NULL;

	name = GDATA_GD_NAME (gdata_parsable_new_from_xml (GDATA_TYPE_GD_NAME,
		"<gd:name xmlns:gd='http://schemas.google.com/g/2005'>"
			"<gd:givenName>Brian</gd:givenName>"
			"<gd:additionalName>Charles</gd:additionalName>"
			"<gd:familyName>Blessed</gd:familyName>"
			"<gd:namePrefix>Mr</gd:namePrefix>"
			"<gd:nameSuffix>ABC</gd:nameSuffix>"
			"<gd:fullName>Mr Brian Charles Blessed, ABC</gd:fullName>"
		"</gd:name>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_NAME (name));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_name_get_given_name (name), ==, "Brian");
	g_assert_cmpstr (gdata_gd_name_get_additional_name (name), ==, "Charles");
	g_assert_cmpstr (gdata_gd_name_get_family_name (name), ==, "Blessed");
	g_assert_cmpstr (gdata_gd_name_get_prefix (name), ==, "Mr");
	g_assert_cmpstr (gdata_gd_name_get_suffix (name), ==, "ABC");
	g_assert_cmpstr (gdata_gd_name_get_full_name (name), ==, "Mr Brian Charles Blessed, ABC");

	/* Compare it against another identical name */
	name2 = gdata_gd_name_new ("Brian", "Blessed");
	gdata_gd_name_set_additional_name (name2, "Charles");
	gdata_gd_name_set_prefix (name2, "Mr");
	gdata_gd_name_set_suffix (name2, "ABC");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (name), GDATA_COMPARABLE (name2)), ==, 0);

	/* …and a different one */
	gdata_gd_name_set_prefix (name2, "Mrs");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (name), GDATA_COMPARABLE (name2)), !=, 0);
	g_object_unref (name2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (name));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:name xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'>"
				"<gd:givenName>Brian</gd:givenName>"
				"<gd:additionalName>Charles</gd:additionalName>"
				"<gd:familyName>Blessed</gd:familyName>"
				"<gd:namePrefix>Mr</gd:namePrefix>"
				"<gd:nameSuffix>ABC</gd:nameSuffix>"
				"<gd:fullName>Mr Brian Charles Blessed, ABC</gd:fullName>"
			 "</gd:name>");
	g_free (xml);
	g_object_unref (name);

	/* Now parse an address with less information available */
	name = GDATA_GD_NAME (gdata_parsable_new_from_xml (GDATA_TYPE_GD_NAME,
		"<gd:name xmlns:gd='http://schemas.google.com/g/2005'><gd:givenName>Bob</gd:givenName></gd:name>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_NAME (name));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_name_get_given_name (name), ==, "Bob");
	g_assert (gdata_gd_name_get_additional_name (name) == NULL);
	g_assert (gdata_gd_name_get_family_name (name) == NULL);
	g_assert (gdata_gd_name_get_prefix (name) == NULL);
	g_assert (gdata_gd_name_get_suffix (name) == NULL);
	g_assert (gdata_gd_name_get_full_name (name) == NULL);

	/* Check the outputted XML is still correct */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (name));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:name xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'>"
				"<gd:givenName>Bob</gd:givenName>"
			 "</gd:name>");
	g_free (xml);
	g_object_unref (name);
}

static void
test_gd_organization (void)
{
	GDataGDOrganization *org, *org2;
	GDataGDWhere *location;
	gchar *xml;
	GError *error = NULL;

	org = GDATA_GD_ORGANIZATION (gdata_parsable_new_from_xml (GDATA_TYPE_GD_ORGANIZATION,
		"<gd:organization xmlns:gd='http://schemas.google.com/g/2005' rel='http://schemas.google.com/g/2005#work' label='Work &amp; Occupation' "
			"primary='true'>"
			"<gd:orgName>Google, Inc.</gd:orgName>"
			"<gd:orgTitle>&lt;Angle Bracketeer&gt;</gd:orgTitle>"
			"<gd:orgDepartment>Finance</gd:orgDepartment>"
			"<gd:orgJobDescription>Doing stuff.</gd:orgJobDescription>"
			"<gd:orgSymbol>FOO</gd:orgSymbol>"
			"<gd:where valueString='Test location'/>"
		"</gd:organization>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_ORGANIZATION (org));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_organization_get_name (org), ==, "Google, Inc.");
	g_assert_cmpstr (gdata_gd_organization_get_title (org), ==, "<Angle Bracketeer>");
	g_assert_cmpstr (gdata_gd_organization_get_relation_type (org), ==, GDATA_GD_ORGANIZATION_WORK);
	g_assert_cmpstr (gdata_gd_organization_get_label (org), ==, "Work & Occupation");
	g_assert_cmpstr (gdata_gd_organization_get_department (org), ==, "Finance");
	g_assert_cmpstr (gdata_gd_organization_get_job_description (org), ==, "Doing stuff.");
	g_assert_cmpstr (gdata_gd_organization_get_symbol (org), ==, "FOO");
	location = gdata_gd_organization_get_location (org);
	g_assert (GDATA_IS_GD_WHERE (location));
	g_assert (gdata_gd_organization_is_primary (org) == TRUE);

	/* Compare it against another identical organization */
	org2 = gdata_gd_organization_new ("Google, Inc.", "<Angle Bracketeer>", GDATA_GD_ORGANIZATION_WORK, "Work & Occupation", TRUE);
	gdata_gd_organization_set_department (org2, "Finance");
	gdata_gd_organization_set_location (org2, location);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (org), GDATA_COMPARABLE (org2)), ==, 0);

	/* …and a different one */
	gdata_gd_organization_set_title (org2, "Demoted!");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (org), GDATA_COMPARABLE (org2)), !=, 0);
	g_object_unref (org2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (org));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:organization xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"rel='http://schemas.google.com/g/2005#work' label='Work &amp; Occupation' primary='true'>"
				"<gd:orgName>Google, Inc.</gd:orgName>"
				"<gd:orgTitle>&lt;Angle Bracketeer&gt;</gd:orgTitle>"
				"<gd:orgDepartment>Finance</gd:orgDepartment>"
				"<gd:orgJobDescription>Doing stuff.</gd:orgJobDescription>"
				"<gd:orgSymbol>FOO</gd:orgSymbol>"
				"<gd:where valueString='Test location'/>"
			 "</gd:organization>");
	g_free (xml);
	g_object_unref (org);

	/* Now parse an organization with less information available */
	org = GDATA_GD_ORGANIZATION (gdata_parsable_new_from_xml (GDATA_TYPE_GD_ORGANIZATION,
		"<gd:organization xmlns:gd='http://schemas.google.com/g/2005'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_ORGANIZATION (org));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gd_organization_get_name (org) == NULL);
	g_assert (gdata_gd_organization_get_title (org) == NULL);
	g_assert (gdata_gd_organization_get_relation_type (org) == NULL);
	g_assert (gdata_gd_organization_get_label (org) == NULL);
	g_assert (gdata_gd_organization_is_primary (org) == FALSE);
	g_assert (gdata_gd_organization_get_department (org) == NULL);
	g_assert (gdata_gd_organization_get_job_description (org) == NULL);
	g_assert (gdata_gd_organization_get_symbol (org) == NULL);
	g_assert (gdata_gd_organization_get_location (org) == NULL);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (org));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:organization xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' primary='false'/>");
	g_free (xml);
	g_object_unref (org);
}

static void
test_gd_organization_escaping (void)
{
	GDataGDOrganization *org;
	gchar *xml;

	org = gdata_gd_organization_new ("Steptoe & Son", "Title & Stuff", GDATA_GD_ORGANIZATION_WORK "?foo&bar", "Personal & Private", TRUE);
	gdata_gd_organization_set_department (org, "Department & Stuff");
	gdata_gd_organization_set_job_description (org, "Escaping <brackets>.");
	gdata_gd_organization_set_symbol (org, "<&>");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (org));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:organization xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                                  "rel='http://schemas.google.com/g/2005#work?foo&amp;bar' label='Personal &amp; Private' primary='true'>"
	                         "<gd:orgName>Steptoe &amp; Son</gd:orgName>"
	                         "<gd:orgTitle>Title &amp; Stuff</gd:orgTitle>"
	                         "<gd:orgDepartment>Department &amp; Stuff</gd:orgDepartment>"
	                         "<gd:orgJobDescription>Escaping &lt;brackets&gt;.</gd:orgJobDescription>"
	                         "<gd:orgSymbol>&lt;&amp;&gt;</gd:orgSymbol>"
	                 "</gd:organization>");
	g_free (xml);
	g_object_unref (org);
}

static void
test_gd_phone_number (void)
{
	GDataGDPhoneNumber *phone, *phone2;
	gchar *xml;
	GError *error = NULL;

	phone = GDATA_GD_PHONE_NUMBER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_PHONE_NUMBER,
		"<gd:phoneNumber xmlns:gd='http://schemas.google.com/g/2005' rel='http://schemas.google.com/g/2005#mobile' "
			"label='Personal &amp; business calls only' uri='tel:+12065551212'>+1 206 555 1212</gd:phoneNumber>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_PHONE_NUMBER (phone));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_phone_number_get_number (phone), ==, "+1 206 555 1212");
	g_assert_cmpstr (gdata_gd_phone_number_get_uri (phone), ==, "tel:+12065551212");
	g_assert_cmpstr (gdata_gd_phone_number_get_relation_type (phone), ==, GDATA_GD_PHONE_NUMBER_MOBILE);
	g_assert_cmpstr (gdata_gd_phone_number_get_label (phone), ==, "Personal & business calls only");
	g_assert (gdata_gd_phone_number_is_primary (phone) == FALSE);

	/* Compare it against another identical number */
	phone2 = gdata_gd_phone_number_new ("+1 206 555 1212", GDATA_GD_PHONE_NUMBER_MOBILE, "Personal & business calls only",
					    "tel:+12065551212", FALSE);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (phone), GDATA_COMPARABLE (phone2)), ==, 0);

	/* …and a different one */
	gdata_gd_phone_number_set_number (phone2, "+1 206 555 1212 666");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (phone), GDATA_COMPARABLE (phone2)), !=, 0);
	g_object_unref (phone2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (phone));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:phoneNumber xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"uri='tel:+12065551212' rel='http://schemas.google.com/g/2005#mobile' label='Personal &amp; business calls only' "
				"primary='false'>+1 206 555 1212</gd:phoneNumber>");
	g_free (xml);

	/* Check we trim whitespace properly, and respect Unicode characters */
	gdata_gd_phone_number_set_number (phone, "  	 0123456 (789) ëxt 300  ");
	g_assert_cmpstr (gdata_gd_phone_number_get_number (phone), ==, "0123456 (789) ëxt 300");
	g_object_unref (phone);

	/* Now parse a phone number with less information available, but some extraneous whitespace */
	phone = GDATA_GD_PHONE_NUMBER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_PHONE_NUMBER,
		"<gd:phoneNumber xmlns:gd='http://schemas.google.com/g/2005'>  (425) 555-8080 ext. 72585  \n </gd:phoneNumber>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_PHONE_NUMBER (phone));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_phone_number_get_number (phone), ==, "(425) 555-8080 ext. 72585");
	g_assert (gdata_gd_phone_number_get_uri (phone) == NULL);
	g_assert (gdata_gd_phone_number_get_relation_type (phone) == NULL);
	g_assert (gdata_gd_phone_number_get_label (phone) == NULL);
	g_assert (gdata_gd_phone_number_is_primary (phone) == FALSE);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (phone));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:phoneNumber xmlns='http://www.w3.org/2005/Atom' "
				"xmlns:gd='http://schemas.google.com/g/2005' primary='false'>(425) 555-8080 ext. 72585</gd:phoneNumber>");
	g_free (xml);
	g_object_unref (phone);
}

static void
test_gd_phone_number_escaping (void)
{
	GDataGDPhoneNumber *phone;
	gchar *xml;

	phone = gdata_gd_phone_number_new ("0123456789 <54>", GDATA_GD_PHONE_NUMBER_WORK_MOBILE "?foo&bar", "Personal & Private",
	                                   "tel:+012345678954?foo&bar", TRUE);

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (phone));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:phoneNumber xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                                 "uri='tel:+012345678954?foo&amp;bar' rel='http://schemas.google.com/g/2005#work_mobile?foo&amp;bar' "
	                                 "label='Personal &amp; Private' primary='true'>0123456789 &lt;54&gt;</gd:phoneNumber>");
	g_free (xml);
	g_object_unref (phone);
}

static void
test_gd_postal_address (void)
{
	GDataGDPostalAddress *postal, *postal2;
	gchar *xml;
	GError *error = NULL;

	postal = GDATA_GD_POSTAL_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_POSTAL_ADDRESS,
		"<gd:structuredPostalAddress xmlns:gd='http://schemas.google.com/g/2005' label='Home &amp; Safe House' "
			"rel='http://schemas.google.com/g/2005#home' primary='true'>"
			"<gd:street>500 West 45th Street</gd:street>"
			"<gd:city>New York</gd:city>"
			"<gd:postcode>NY 10036</gd:postcode>"
			"<gd:country code='US'>United States</gd:country>"
		"</gd:structuredPostalAddress>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_POSTAL_ADDRESS (postal));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_postal_address_get_street (postal), ==, "500 West 45th Street");
	g_assert_cmpstr (gdata_gd_postal_address_get_city (postal), ==, "New York");
	g_assert_cmpstr (gdata_gd_postal_address_get_postcode (postal), ==, "NY 10036");
	g_assert_cmpstr (gdata_gd_postal_address_get_relation_type (postal), ==, GDATA_GD_POSTAL_ADDRESS_HOME);
	g_assert_cmpstr (gdata_gd_postal_address_get_label (postal), ==, "Home & Safe House");
	g_assert_cmpstr (gdata_gd_postal_address_get_country (postal), ==, "United States");
	g_assert_cmpstr (gdata_gd_postal_address_get_country_code (postal), ==, "US");
	g_assert (gdata_gd_postal_address_is_primary (postal) == TRUE);

	/* Compare it against another identical address */
	postal2 = gdata_gd_postal_address_new (GDATA_GD_POSTAL_ADDRESS_HOME, "Home & Safe House", TRUE);
	gdata_gd_postal_address_set_street (postal2, "500 West 45th Street");
	gdata_gd_postal_address_set_city (postal2, "New York");
	gdata_gd_postal_address_set_postcode (postal2, "NY 10036");
	gdata_gd_postal_address_set_country (postal2, "United States", "US");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (postal), GDATA_COMPARABLE (postal2)), ==, 0);

	/* …and a different one */
	gdata_gd_postal_address_set_city (postal2, "Atlas Mountains");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (postal), GDATA_COMPARABLE (postal2)), !=, 0);
	g_object_unref (postal2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (postal));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:structuredPostalAddress xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"rel='http://schemas.google.com/g/2005#home' label='Home &amp; Safe House' primary='true'>"
				"<gd:street>500 West 45th Street</gd:street>"
				"<gd:city>New York</gd:city>"
				"<gd:postcode>NY 10036</gd:postcode>"
				"<gd:country code='US'>United States</gd:country>"
			 "</gd:structuredPostalAddress>");
	g_free (xml);

	/* Check we trim whitespace properly, and respect Unicode characters */
	gdata_gd_postal_address_set_address (postal, "  	 Schöne Grüße Straße\nGermany  ");
	g_assert_cmpstr (gdata_gd_postal_address_get_address (postal), ==, "Schöne Grüße Straße\nGermany");
	g_object_unref (postal);

	/* Now parse an address with less information available */
	postal = GDATA_GD_POSTAL_ADDRESS (gdata_parsable_new_from_xml (GDATA_TYPE_GD_POSTAL_ADDRESS,
		"<gd:structuredPostalAddress xmlns:gd='http://schemas.google.com/g/2005'><gd:street>f</gd:street></gd:structuredPostalAddress>",
		-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_POSTAL_ADDRESS (postal));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_postal_address_get_street (postal), ==, "f");
	g_assert (gdata_gd_postal_address_get_relation_type (postal) == NULL);
	g_assert (gdata_gd_postal_address_get_label (postal) == NULL);
	g_assert (gdata_gd_postal_address_is_primary (postal) == FALSE);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (postal));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:structuredPostalAddress xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' primary='false'>"
				 "<gd:street>f</gd:street></gd:structuredPostalAddress>");
	g_free (xml);
	g_object_unref (postal);
}

static void
test_gd_postal_address_escaping (void)
{
	GDataGDPostalAddress *address;
	gchar *xml;

	address = gdata_gd_postal_address_new (GDATA_GD_POSTAL_ADDRESS_WORK "?foo&bar", "Personal & Private", TRUE);
	gdata_gd_postal_address_set_address (address, "<address>");
	gdata_gd_postal_address_set_mail_class (address, GDATA_GD_MAIL_CLASS_BOTH "?foo&bar");
	gdata_gd_postal_address_set_usage (address, GDATA_GD_ADDRESS_USAGE_GENERAL "?foo&bar");
	gdata_gd_postal_address_set_agent (address, "<agent>");
	gdata_gd_postal_address_set_house_name (address, "House & House");
	gdata_gd_postal_address_set_street (address, "Church & Main Street");
	gdata_gd_postal_address_set_po_box (address, "<515>");
	gdata_gd_postal_address_set_neighborhood (address, "<neighbourhood>");
	gdata_gd_postal_address_set_city (address, "City <17>");
	gdata_gd_postal_address_set_subregion (address, "Subregion <5>");
	gdata_gd_postal_address_set_region (address, "<region>");
	gdata_gd_postal_address_set_postcode (address, "Postcode & stuff");
	gdata_gd_postal_address_set_country (address, "<foo>", "<bar>");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (address));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:structuredPostalAddress xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                                             "rel='http://schemas.google.com/g/2005#work?foo&amp;bar' label='Personal &amp; Private' "
	                                             "mailClass='http://schemas.google.com/g/2005#both?foo&amp;bar' "
	                                             "usage='http://schemas.google.com/g/2005#general?foo&amp;bar' primary='true'>"
	                         "<gd:agent>&lt;agent&gt;</gd:agent>"
	                         "<gd:housename>House &amp; House</gd:housename>"
	                         "<gd:street>Church &amp; Main Street</gd:street>"
	                         "<gd:pobox>&lt;515&gt;</gd:pobox>"
	                         "<gd:neighborhood>&lt;neighbourhood&gt;</gd:neighborhood>"
	                         "<gd:city>City &lt;17&gt;</gd:city>"
	                         "<gd:subregion>Subregion &lt;5&gt;</gd:subregion>"
	                         "<gd:region>&lt;region&gt;</gd:region>"
	                         "<gd:postcode>Postcode &amp; stuff</gd:postcode>"
	                         "<gd:country code='&lt;bar&gt;'>&lt;foo&gt;</gd:country>"
	                         "<gd:formattedAddress>&lt;address&gt;</gd:formattedAddress>"
	                 "</gd:structuredPostalAddress>");
	g_free (xml);
	g_object_unref (address);
}

static void
test_gd_reminder (void)
{
	GDataGDReminder *reminder, *reminder2;
	gchar *xml;
	GTimeVal time_val;
	GError *error = NULL;

	reminder = GDATA_GD_REMINDER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_REMINDER,
		"<gd:reminder xmlns:gd='http://schemas.google.com/g/2005' days='15'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_REMINDER (reminder));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gd_reminder_get_method (reminder) == NULL);
	g_assert (gdata_gd_reminder_is_absolute_time (reminder) == FALSE);
	g_assert_cmpint (gdata_gd_reminder_get_relative_time (reminder), ==, 15 * 24 * 60);

	/* Check the outputted XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (reminder));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:reminder xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' minutes='21600'/>");
	g_free (xml);
	g_object_unref (reminder);

	/* Try again with a different property */
	reminder = GDATA_GD_REMINDER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_REMINDER,
		"<gd:reminder xmlns:gd='http://schemas.google.com/g/2005' hours='15'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_REMINDER (reminder));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gd_reminder_get_method (reminder) == NULL);
	g_assert (gdata_gd_reminder_is_absolute_time (reminder) == FALSE);
	g_assert_cmpint (gdata_gd_reminder_get_relative_time (reminder), ==, 15 * 60);

	/* Compare to another reminder */
	reminder2 = gdata_gd_reminder_new (NULL, NULL, 15 * 60);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (reminder), GDATA_COMPARABLE (reminder2)), ==, 0);
	g_object_unref (reminder2);
	g_object_unref (reminder);

	/* …and another */
	reminder = GDATA_GD_REMINDER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_REMINDER,
		"<gd:reminder xmlns:gd='http://schemas.google.com/g/2005' minutes='15'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_REMINDER (reminder));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gd_reminder_get_method (reminder) == NULL);
	g_assert (gdata_gd_reminder_is_absolute_time (reminder) == FALSE);
	g_assert_cmpint (gdata_gd_reminder_get_relative_time (reminder), ==, 15);
	g_object_unref (reminder);

	/* Try again with an absolute time and a method */
	reminder = GDATA_GD_REMINDER (gdata_parsable_new_from_xml (GDATA_TYPE_GD_REMINDER,
		"<gd:reminder xmlns:gd='http://schemas.google.com/g/2005' method='alert' absoluteTime='2005-06-06T16:55:00-08:00'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_REMINDER (reminder));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_reminder_get_method (reminder), ==, GDATA_GD_REMINDER_ALERT);
	g_assert (gdata_gd_reminder_is_absolute_time (reminder) == TRUE);
	gdata_gd_reminder_get_absolute_time (reminder, &time_val);
	g_assert_cmpint (time_val.tv_sec, ==, 1118105700);
	g_assert_cmpint (time_val.tv_usec, ==, 0);

	/* Compare to another reminder */
	reminder2 = gdata_gd_reminder_new (GDATA_GD_REMINDER_ALERT, &time_val, -1);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (reminder), GDATA_COMPARABLE (reminder2)), ==, 0);
	g_object_unref (reminder2);

	/* Check the outputted XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (reminder));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:reminder xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"absoluteTime='2005-06-07T00:55:00Z' method='alert'/>");
	g_free (xml);
	g_object_unref (reminder);
}

static void
test_gd_reminder_escaping (void)
{
	GDataGDReminder *reminder;
	gchar *xml;

	reminder = gdata_gd_reminder_new (GDATA_GD_REMINDER_ALERT "?foo&bar", NULL, 15);

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (reminder));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:reminder xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                              "minutes='15' method='alert?foo&amp;bar'/>");
	g_free (xml);
	g_object_unref (reminder);
}

static void
test_gd_when (void)
{
	GDataGDWhen *when, *when2;
	GDataGDReminder *reminder;
	gchar *xml;
	GList *reminders;
	GTimeVal time_val, time_val2;
	GError *error = NULL;

	when = GDATA_GD_WHEN (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHEN,
		"<gd:when xmlns:gd='http://schemas.google.com/g/2005' startTime='2005-06-06T17:00:00-08:00' endTime='2005-06-06T18:00:00-08:00'/>",
		-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHEN (when));
	g_clear_error (&error);

	/* Check the properties */
	gdata_gd_when_get_start_time (when, &time_val);
	g_assert_cmpint (time_val.tv_sec, ==, 1118106000);
	g_assert_cmpint (time_val.tv_usec, ==, 0);
	gdata_gd_when_get_end_time (when, &time_val2);
	g_assert_cmpint (time_val2.tv_sec, ==, 1118109600);
	g_assert_cmpint (time_val2.tv_usec, ==, 0);
	g_assert (gdata_gd_when_is_date (when) == FALSE);
	g_assert (gdata_gd_when_get_value_string (when) == NULL);
	g_assert (gdata_gd_when_get_reminders (when) == NULL);

	/* Compare it against another identical time */
	when2 = gdata_gd_when_new (&time_val, &time_val2, FALSE);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (when), GDATA_COMPARABLE (when2)), ==, 0);

	/* …and a different one */
	time_val2.tv_usec = 100;
	gdata_gd_when_set_end_time (when2, &time_val2);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (when), GDATA_COMPARABLE (when2)), !=, 0);
	g_object_unref (when2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (when));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:when xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' startTime='2005-06-07T01:00:00Z' "
				"endTime='2005-06-07T02:00:00Z'/>");
	g_free (xml);
	g_object_unref (when);

	/* Now parse a time with different information */
	when = GDATA_GD_WHEN (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHEN,
		"<gd:when xmlns:gd='http://schemas.google.com/g/2005' startTime='2005-06-06' endTime='2005-06-08' valueString='This weekend'>"
			"<gd:reminder minutes='15'/>"
			"<foobar/>"
		"</gd:when>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHEN (when));
	g_clear_error (&error);

	/* Check the properties */
	gdata_gd_when_get_start_time (when, &time_val);
	g_assert_cmpint (time_val.tv_sec, ==, 1118016000);
	g_assert_cmpint (time_val.tv_usec, ==, 0);
	gdata_gd_when_get_end_time (when, &time_val2);
	g_assert_cmpint (time_val2.tv_sec, ==, 1118188800);
	g_assert_cmpint (time_val2.tv_usec, ==, 0);
	g_assert (gdata_gd_when_is_date (when) == TRUE);
	g_assert_cmpstr (gdata_gd_when_get_value_string (when), ==, "This weekend");

	reminders = gdata_gd_when_get_reminders (when);
	g_assert (reminders != NULL);
	g_assert (GDATA_IS_GD_REMINDER (reminders->data));
	g_assert (reminders->next == NULL);
	g_assert (gdata_gd_reminder_is_absolute_time (GDATA_GD_REMINDER (reminders->data)) == FALSE);
	g_assert_cmpint (gdata_gd_reminder_get_relative_time (GDATA_GD_REMINDER (reminders->data)), ==, 15);

	/* Add another reminder */
	reminder = gdata_gd_reminder_new (GDATA_GD_REMINDER_ALERT, &time_val, -1);
	gdata_gd_when_add_reminder (when, reminder);
	g_object_unref (reminder);

	/* Check the outputted XML is correct */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (when));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:when xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' startTime='2005-06-06' "
				"endTime='2005-06-08' valueString='This weekend'>"
				"<gd:reminder minutes='15'/>"
				"<gd:reminder absoluteTime='2005-06-06T00:00:00Z' method='alert'/>"
				"<foobar/>"
			 "</gd:when>");
	g_free (xml);
	g_object_unref (when);
}

static void
test_gd_when_escaping (void)
{
	GDataGDWhen *when;
	gchar *xml;
	GTimeVal start_time;

	g_time_val_from_iso8601 ("2005-06-07T01:00:00Z", &start_time);
	when = gdata_gd_when_new (&start_time, NULL, FALSE);
	gdata_gd_when_set_value_string (when, "Value string & stuff!");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (when));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:when xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                          "startTime='2005-06-07T01:00:00Z' valueString='Value string &amp; stuff!'/>");
	g_free (xml);
	g_object_unref (when);
}

static void
test_gd_where (void)
{
	GDataGDWhere *where, *where2;
	gchar *xml;
	GError *error = NULL;

	where = GDATA_GD_WHERE (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHERE,
		"<gd:where xmlns:gd='http://schemas.google.com/g/2005' rel='http://schemas.google.com/g/2005#event.alternate' "
			"label='New York Location &lt;videoconference&gt;' valueString='Metropolis'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHERE (where));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_where_get_relation_type (where), ==, GDATA_GD_WHERE_EVENT_ALTERNATE);
	g_assert_cmpstr (gdata_gd_where_get_value_string (where), ==, "Metropolis");
	g_assert_cmpstr (gdata_gd_where_get_label (where), ==, "New York Location <videoconference>");

	/* Compare it against another identical place */
	where2 = gdata_gd_where_new (GDATA_GD_WHERE_EVENT_ALTERNATE, "Metropolis", "New York Location <videoconference>");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (where), GDATA_COMPARABLE (where2)), ==, 0);

	/* …and a different one */
	gdata_gd_where_set_label (where2, "Atlas Mountains");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (where), GDATA_COMPARABLE (where2)), !=, 0);
	g_object_unref (where2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (where));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:where xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"label='New York Location &lt;videoconference&gt;' rel='http://schemas.google.com/g/2005#event.alternate' "
				"valueString='Metropolis'/>");
	g_free (xml);
	g_object_unref (where);

	/* Now parse a place with less information available */
	where = GDATA_GD_WHERE (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHERE,
		"<gd:where xmlns:gd='http://schemas.google.com/g/2005' valueString='Google Cafeteria &lt;Building 40&gt;'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHERE (where));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_where_get_value_string (where), ==, "Google Cafeteria <Building 40>");
	g_assert (gdata_gd_where_get_relation_type (where) == NULL);
	g_assert (gdata_gd_where_get_label (where) == NULL);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (where));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:where xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
				"valueString='Google Cafeteria &lt;Building 40&gt;'/>");
	g_free (xml);
	g_object_unref (where);
}

static void
test_gd_where_escaping (void)
{
	GDataGDWhere *where;
	gchar *xml;

	where = gdata_gd_where_new (GDATA_GD_WHERE_EVENT "?foo&bar", "Value string & stuff!", "<label>");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (where));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:where xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                           "label='&lt;label&gt;' rel='http://schemas.google.com/g/2005#event?foo&amp;bar' "
	                           "valueString='Value string &amp; stuff!'/>");
	g_free (xml);
	g_object_unref (where);
}

static void
test_gd_who (void)
{
	GDataGDWho *who, *who2;
	gchar *xml;
	GError *error = NULL;

	who = GDATA_GD_WHO (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHO,
		"<gd:who xmlns:gd='http://schemas.google.com/g/2005' rel='http://schemas.google.com/g/2005#message.to' valueString='Elizabeth' "
			"email='liz@example.com'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHO (who));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gd_who_get_relation_type (who), ==, "http://schemas.google.com/g/2005#message.to");
	g_assert_cmpstr (gdata_gd_who_get_value_string (who), ==, "Elizabeth");
	g_assert_cmpstr (gdata_gd_who_get_email_address (who), ==, "liz@example.com");

	/* Compare it against another identical person */
	who2 = gdata_gd_who_new ("http://schemas.google.com/g/2005#message.to", "Elizabeth", "liz@example.com");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (who), GDATA_COMPARABLE (who2)), ==, 0);

	/* …and a different one */
	gdata_gd_who_set_email_address (who2, "john@example.com");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (who), GDATA_COMPARABLE (who2)), !=, 0);
	g_object_unref (who2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (who));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:who xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' email='liz@example.com' "
				"rel='http://schemas.google.com/g/2005#message.to' valueString='Elizabeth'/>");
	g_free (xml);
	g_object_unref (who);

	/* Now parse a place with less information available */
	who = GDATA_GD_WHO (gdata_parsable_new_from_xml (GDATA_TYPE_GD_WHO,
		"<gd:who xmlns:gd='http://schemas.google.com/g/2005'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GD_WHO (who));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gd_who_get_value_string (who) == NULL);
	g_assert (gdata_gd_who_get_relation_type (who) == NULL);
	g_assert (gdata_gd_who_get_email_address (who) == NULL);

	/* Check the outputted XML contains the unknown XML */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (who));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gd:who xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'/>");
	g_free (xml);
	g_object_unref (who);
}

static void
test_gd_who_escaping (void)
{
	GDataGDWho *who;
	gchar *xml;

	who = gdata_gd_who_new (GDATA_GD_WHO_EVENT_ATTENDEE "?foo&bar", "Value string & stuff!", "John Smith <john.smith@gmail.com>");

	/* Check the outputted XML is escaped properly */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (who));
	g_assert_cmpstr (xml, ==,
	                 "<?xml version='1.0' encoding='UTF-8'?>"
	                 "<gd:who xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
	                         "email='John Smith &lt;john.smith@gmail.com&gt;' rel='http://schemas.google.com/g/2005#event.attendee?foo&amp;bar' "
	                         "valueString='Value string &amp; stuff!'/>");
	g_free (xml);
	g_object_unref (who);
}

static void
test_media_category (void)
{
	GDataMediaCategory *category;
	gchar *xml;
	GError *error = NULL;

	category = GDATA_MEDIA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CATEGORY,
		"<media:category xmlns:media='http://search.yahoo.com/mrss/' scheme='http://dmoz.org' "
			"label='Ace Ventura - Pet &amp; Detective'>Arts/Movies/Titles/A/Ace_Ventura_Series/Ace_Ventura_-_Pet_Detective"
			"</media:category>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CATEGORY (category));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_category_get_category (category), ==, "Arts/Movies/Titles/A/Ace_Ventura_Series/Ace_Ventura_-_Pet_Detective");
	g_assert_cmpstr (gdata_media_category_get_scheme (category), ==, "http://dmoz.org");
	g_assert_cmpstr (gdata_media_category_get_label (category), ==, "Ace Ventura - Pet & Detective");

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (category));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<media:category xmlns='http://www.w3.org/2005/Atom' xmlns:media='http://search.yahoo.com/mrss/' "
				"scheme='http://dmoz.org' "
				"label='Ace Ventura - Pet &amp; Detective'>Arts/Movies/Titles/A/Ace_Ventura_Series/Ace_Ventura_-_Pet_Detective"
				"</media:category>");
	g_free (xml);
	g_object_unref (category);

	/* Now parse one with less information available */
	category = GDATA_MEDIA_CATEGORY (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CATEGORY,
		"<media:category xmlns:media='http://search.yahoo.com/mrss/'>foo</media:category>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CATEGORY (category));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_category_get_category (category), ==, "foo");
	g_assert_cmpstr (gdata_media_category_get_scheme (category), ==, "http://video.search.yahoo.com/mrss/category_schema");
	g_assert (gdata_media_category_get_label (category) == NULL);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (category));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<media:category xmlns='http://www.w3.org/2005/Atom' xmlns:media='http://search.yahoo.com/mrss/' "
				"scheme='http://video.search.yahoo.com/mrss/category_schema'>foo</media:category>");
	g_free (xml);
	g_object_unref (category);
}

static void
test_media_content (void)
{
	GDataMediaContent *content;
	GError *error = NULL;

	content = GDATA_MEDIA_CONTENT (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CONTENT,
		"<media:content xmlns:media='http://search.yahoo.com/mrss/' url='http://www.foo.com/movie.mov' fileSize='12216320' "
			"type='video/quicktime' medium='video' isDefault='true' expression='nonstop' duration='185' height='200' width='300'/>",
			-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CONTENT (content));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_content_get_uri (content), ==, "http://www.foo.com/movie.mov");
	g_assert_cmpint (gdata_media_content_get_filesize (content), ==, 12216320);
	g_assert_cmpstr (gdata_media_content_get_content_type (content), ==, "video/quicktime");
	g_assert (gdata_media_content_get_medium (content) == GDATA_MEDIA_VIDEO);
	g_assert (gdata_media_content_is_default (content) == TRUE);
	g_assert (gdata_media_content_get_expression (content) == GDATA_MEDIA_EXPRESSION_NONSTOP);
	g_assert_cmpint (gdata_media_content_get_duration (content), ==, 185);
	g_assert_cmpuint (gdata_media_content_get_width (content), ==, 300);
	g_assert_cmpuint (gdata_media_content_get_height (content), ==, 200);

	/* NOTE: We don't check the outputted XML, since the class currently doesn't have any support for outputting XML */
	g_object_unref (content);

	/* Now parse one with less information available */
	content = GDATA_MEDIA_CONTENT (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CONTENT,
		"<media:content xmlns:media='http://search.yahoo.com/mrss/' url='http://foobar.com/'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CONTENT (content));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_content_get_uri (content), ==, "http://foobar.com/");
	g_assert_cmpint (gdata_media_content_get_filesize (content), ==, 0);
	g_assert (gdata_media_content_get_content_type (content) == NULL);
	g_assert (gdata_media_content_get_medium (content) == GDATA_MEDIA_UNKNOWN);
	g_assert (gdata_media_content_is_default (content) == FALSE);
	g_assert (gdata_media_content_get_expression (content) == GDATA_MEDIA_EXPRESSION_FULL);
	g_assert_cmpint (gdata_media_content_get_duration (content), ==, 0);
	g_assert_cmpuint (gdata_media_content_get_width (content), ==, 0);
	g_assert_cmpuint (gdata_media_content_get_height (content), ==, 0);

	g_object_unref (content);
}

static void
test_media_credit (void)
{
	GDataMediaCredit *credit;
	GError *error = NULL;

	credit = GDATA_MEDIA_CREDIT (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CREDIT,
		"<media:credit xmlns:media='http://search.yahoo.com/mrss/' role='producer' scheme='urn:foobar'>entity name</media:credit>",
			-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CREDIT (credit));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_credit_get_credit (credit), ==, "entity name");
	g_assert_cmpstr (gdata_media_credit_get_scheme (credit), ==, "urn:foobar");
	g_assert_cmpstr (gdata_media_credit_get_role (credit), ==, "producer");

	/* NOTE: We don't check the outputted XML, since the class currently doesn't have any support for outputting XML */
	g_object_unref (credit);

	/* Now parse one with less information available */
	credit = GDATA_MEDIA_CREDIT (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_CREDIT,
		"<media:credit xmlns:media='http://search.yahoo.com/mrss/'>John Smith</media:credit>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_CREDIT (credit));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_credit_get_credit (credit), ==, "John Smith");
	g_assert_cmpstr (gdata_media_credit_get_scheme (credit), ==, "urn:ebu");
	g_assert (gdata_media_credit_get_role (credit) == NULL);

	g_object_unref (credit);
}

#if 0
/* We can't test GDataMediaGroup, since it isn't currently publicly exposed */
static void
test_media_group (void)
{
	GDataMediaGroup *group;
	GList *contents, *thumbnails;
	gchar *xml;
	GError *error = NULL;

	group = GDATA_MEDIA_GROUP (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_GROUP,
		"<media:group xmlns:media='http://search.yahoo.com/mrss/'>"
			"<media:title>Foobar — shizzle!</media:title>"
			"<media:description>This is a description, isn't it‽</media:description>"
			"<media:keywords>keywords,are, fun</media:keywords>"
			"<media:category scheme='http://dmoz.org' label='Ace Ventura - Pet Detective'>"
				"Arts/Movies/Titles/A/Ace_Ventura_Series/Ace_Ventura_-_Pet_Detective</media:category>"
			"<media:content url='http://foobar.com/'/>"
			"<media:content url='http://www.foo.com/movie.mov' fileSize='12216320' type='video/quicktime' medium='video' isDefault='true' "
				"expression='nonstop' duration='185' height='200' width='300'/>"
			"<media:credit>John Smith</media:credit>"
			"<media:player url='http://www.foo.com/player?id=1111' height='200' width='400'/>"
			"<media:restriction relationship='deny'>all</media:restriction>"
			"<media:restriction relationship='allow' type='country'>au us</media:restriction>"
			"<media:thumbnail url='http://www.foo.com/keyframe.jpg' width='75' height='50' time='12:05:01.123'/>"
			"<media:thumbnail url='http://www.foo.com/keyframe0.jpg' time='00:00:00'/>"
		"</media:group>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_GROUP (group));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_group_get_title (group), ==, "Foobar — shizzle!");
	g_assert_cmpstr (gdata_media_group_get_description (group), ==, "This is a description, isn't it‽");
	g_assert_cmpstr (gdata_media_group_get_keywords (group), ==, "keywords,are, fun");
	g_assert (GDATA_IS_MEDIA_CATEGORY (gdata_media_group_get_category (group)));
	g_assert (GDATA_IS_MEDIA_CONTENT (gdata_media_group_look_up_content (group, NULL)));
	g_assert (GDATA_IS_MEDIA_CONTENT (gdata_media_group_look_up_content (group, "video/quicktime")));

	contents = gdata_media_group_get_contents (group);
	g_assert_cmpuint (g_list_length (contents), ==, 2);
	g_assert (GDATA_IS_MEDIA_CONTENT (contents->data));
	g_assert (GDATA_IS_MEDIA_CONTENT (contents->next->data));

	g_assert (GDATA_IS_MEDIA_CREDIT (gdata_media_group_get_credit (group)));
	g_assert_cmpstr (gdata_media_group_get_player_uri (group), ==, "http://www.foo.com/player?id=1111");
	g_assert (gdata_media_group_is_restricted_in_country (group, "uk") == TRUE);
	g_assert (gdata_media_group_is_restricted_in_country (group, "au") == FALSE);
	g_assert (gdata_media_group_is_restricted_in_country (group, "us") == FALSE);

	thumbnails = gdata_media_group_get_thumbnails (group);
	g_assert_cmpuint (g_list_length (thumbnails), ==, 2);
	g_assert (GDATA_IS_MEDIA_THUMBNAIL (thumbnails->data));
	g_assert (GDATA_IS_MEDIA_THUMBNAIL (thumbnails->next->data));

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (group));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<media:group xmlns:media='http://search.yahoo.com/mrss/'>"
				"<media:title>Foobar — shizzle!</media:title>"
				"<media:description>This is a description, isn't it‽</media:description>"
				"<media:keywords>keywords,are, fun</media:keywords>"
				"<media:category scheme='http://dmoz.org' label='Ace Ventura - Pet Detective'>"
					"Arts/Movies/Titles/A/Ace_Ventura_Series/Ace_Ventura_-_Pet_Detective</media:category>"
				"<media:content url='http://foobar.com/'/>"
				"<media:content url='http://www.foo.com/movie.mov' fileSize='12216320' type='video/quicktime' medium='video' "
					"isDefault='true' expression='nonstop' duration='185' height='200' width='300'/>"
				"<media:credit>John Smith</media:credit>"
				"<media:player url='http://www.foo.com/player?id=1111' height='200' width='400'/>"
				"<media:restriction relationship='deny'>all</media:restriction>"
				"<media:restriction relationship='allow' type='country'>au us</media:restriction>"
				"<media:thumbnail url='http://www.foo.com/keyframe.jpg' width='75' height='50' time='12:05:01.123'/>"
				"<media:thumbnail url='http://www.foo.com/keyframe0.jpg' time='00:00:00'/>"
			 "</media:group>");
	g_free (xml);

	/* Check setting things works */
	gdata_media_group_set_title (group, "Test title");
	g_assert_cmpstr (gdata_media_group_get_title (group), ==, "Test title");
	gdata_media_group_set_title (group, NULL);
	g_assert (gdata_media_group_get_title (group) == NULL);

	gdata_media_group_set_description (group, "Foobar");
	g_assert_cmpstr (gdata_media_group_get_description (group), ==, "Foobar");
	gdata_media_group_set_description (group, NULL);
	g_assert (gdata_media_group_get_description (group) == NULL);

	gdata_media_group_set_keywords (group, "a,b, c");
	g_assert_cmpstr (gdata_media_group_get_keywords (group), ==, "a,b, c");
	gdata_media_group_set_keywords (group, NULL);
	g_assert (gdata_media_group_get_keywords (group) == NULL);

	gdata_media_group_set_category (group, NULL);
	g_assert (gdata_media_group_get_category (group) == NULL);

	g_object_unref (group);

	/* Now parse one with less information available */
	group = GDATA_MEDIA_GROUP (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_GROUP,
		"<media:group xmlns:media='http://search.yahoo.com/mrss/'></media:group>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_GROUP (group));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_media_group_get_title (group) == NULL);
	g_assert (gdata_media_group_get_description (group) == NULL);
	g_assert (gdata_media_group_get_keywords (group) == NULL);
	g_assert (gdata_media_group_get_category (group) == NULL);
	g_assert (gdata_media_group_look_up_content (group, NULL) == NULL);
	g_assert (gdata_media_group_look_up_content (group, "video/quicktime") == NULL);
	g_assert (gdata_media_group_get_contents (group) == NULL);
	g_assert (gdata_media_group_get_credit (group) == NULL);
	g_assert (gdata_media_group_get_player_uri (group) == NULL);
	g_assert (gdata_media_group_is_restricted_in_country (group, "uk") == FALSE);
	g_assert (gdata_media_group_is_restricted_in_country (group, "au") == FALSE);
	g_assert (gdata_media_group_is_restricted_in_country (group, "us") == FALSE);
	g_assert (gdata_media_group_get_thumbnails (group) == NULL);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (group));
	g_assert_cmpstr (xml, ==, "<?xml version='1.0' encoding='UTF-8'?>"
	                          "<media:group xmlns='http://www.w3.org/2005/Atom' xmlns:media='http://search.yahoo.com/mrss/'></media:group>");
	g_free (xml);
	g_object_unref (group);
}
#endif

static void
test_media_thumbnail (void)
{
	GDataMediaThumbnail *thumbnail;
	GError *error = NULL;

	thumbnail = GDATA_MEDIA_THUMBNAIL (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_THUMBNAIL,
		"<media:thumbnail xmlns:media='http://search.yahoo.com/mrss/' url='http://www.foo.com/keyframe.jpg' width='75' height='50' "
			"time='12:05:01.123'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_THUMBNAIL (thumbnail));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_thumbnail_get_uri (thumbnail), ==, "http://www.foo.com/keyframe.jpg");
	g_assert_cmpuint (gdata_media_thumbnail_get_width (thumbnail), ==, 75);
	g_assert_cmpuint (gdata_media_thumbnail_get_height (thumbnail), ==, 50);
	g_assert_cmpint (gdata_media_thumbnail_get_time (thumbnail), ==, 43501123);

	/* NOTE: We don't check the outputted XML, since the class currently doesn't have any support for outputting XML */
	g_object_unref (thumbnail);

	/* Now parse one with less information available */
	thumbnail = GDATA_MEDIA_THUMBNAIL (gdata_parsable_new_from_xml (GDATA_TYPE_MEDIA_THUMBNAIL,
		"<media:thumbnail xmlns:media='http://search.yahoo.com/mrss/' url='http://foobar.com/'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_MEDIA_THUMBNAIL (thumbnail));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_media_thumbnail_get_uri (thumbnail), ==, "http://foobar.com/");
	g_assert_cmpuint (gdata_media_thumbnail_get_width (thumbnail), ==, 0);
	g_assert_cmpuint (gdata_media_thumbnail_get_height (thumbnail), ==, 0);
	g_assert_cmpint (gdata_media_thumbnail_get_time (thumbnail), ==, -1);

	g_object_unref (thumbnail);
}

static void
test_gcontact_calendar (void)
{
	GDataGContactCalendar *calendar, *calendar2;
	gchar *xml;
	GError *error = NULL;

	calendar = GDATA_GCONTACT_CALENDAR (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_CALENDAR,
		"<gContact:calendarLink xmlns:gContact='http://schemas.google.com/contact/2008' rel='work' primary='true' "
			"href='http://calendar.com/'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_CALENDAR (calendar));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_calendar_get_uri (calendar), ==, "http://calendar.com/");
	g_assert_cmpstr (gdata_gcontact_calendar_get_relation_type (calendar), ==, GDATA_GCONTACT_CALENDAR_WORK);
	g_assert (gdata_gcontact_calendar_get_label (calendar) == NULL);
	g_assert (gdata_gcontact_calendar_is_primary (calendar) == TRUE);

	/* Compare it against another identical calendar */
	calendar2 = gdata_gcontact_calendar_new ("http://calendar.com/", GDATA_GCONTACT_CALENDAR_WORK, NULL, TRUE);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (calendar), GDATA_COMPARABLE (calendar2)), ==, 0);

	/* …and a different one */
	gdata_gcontact_calendar_set_uri (calendar2, "http://calendar.somewhereelse.com/");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (calendar), GDATA_COMPARABLE (calendar2)), !=, 0);
	g_object_unref (calendar2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (calendar));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:calendarLink xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"href='http://calendar.com/' rel='work' primary='true'/>");
	g_free (xml);
	g_object_unref (calendar);

	/* Now parse a calendar with less information available */
	calendar = GDATA_GCONTACT_CALENDAR (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_CALENDAR,
		"<gContact:calendarLink xmlns:gContact='http://schemas.google.com/contact/2008' href='http://example.com/' label='&lt;a&gt;'/>",
		-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_CALENDAR (calendar));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_calendar_get_uri (calendar), ==, "http://example.com/");
	g_assert (gdata_gcontact_calendar_get_relation_type (calendar) == NULL);
	g_assert_cmpstr (gdata_gcontact_calendar_get_label (calendar), ==, "<a>");
	g_assert (gdata_gcontact_calendar_is_primary (calendar) == FALSE);

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (calendar));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:calendarLink xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"href='http://example.com/' label='&lt;a&gt;' primary='false'/>");
	g_free (xml);
	g_object_unref (calendar);
}

static void
test_gcontact_calendar_error_handling (void)
{
	GDataGContactCalendar *calendar;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) calendar = GDATA_GCONTACT_CALENDAR (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_CALENDAR,\
		"<gContact:calendarLink xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"/>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (calendar == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING ("rel='work'"); /* no href */
	TEST_XML_ERROR_HANDLING ("rel='work' href=''"); /* empty href */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/'"); /* no rel or label */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel=''"); /* empty rel */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' label=''"); /* empty label */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel='other' label='Other'"); /* rel and label */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel='home' primary='not a boolean'"); /* invalid primary */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_event (void)
{
	GDataGContactEvent *event, *event2;
	GDate date;
	gchar *xml;
	GError *error = NULL;

	event = GDATA_GCONTACT_EVENT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EVENT,
		"<gContact:event xmlns:gContact='http://schemas.google.com/contact/2008' xmlns:gd='http://schemas.google.com/g/2005' rel='other'>"
			"<gd:when startTime='2004-03-12'/>"
		"</gContact:event>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_EVENT (event));
	g_clear_error (&error);

	/* Check the properties */
	gdata_gcontact_event_get_date (event, &date);
	g_assert (g_date_valid (&date) == TRUE);
	g_assert_cmpuint (g_date_get_year (&date), ==, 2004);
	g_assert_cmpuint (g_date_get_month (&date), ==, 3);
	g_assert_cmpuint (g_date_get_day (&date), ==, 12);
	g_assert_cmpstr (gdata_gcontact_event_get_relation_type (event), ==, GDATA_GCONTACT_EVENT_OTHER);
	g_assert (gdata_gcontact_event_get_label (event) == NULL);

	/* Try creating another event from scratch */
	event2 = gdata_gcontact_event_new (&date, GDATA_GCONTACT_EVENT_OTHER, NULL);
	g_assert (GDATA_IS_GCONTACT_EVENT (event2));
	g_object_unref (event2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (event));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:event xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
			   "xmlns:gContact='http://schemas.google.com/contact/2008' rel='other'>"
				"<gd:when startTime='2004-03-12'/>"
			 "</gContact:event>");
	g_free (xml);
	g_object_unref (event);

	/* Now parse an event with different information available */
	event = GDATA_GCONTACT_EVENT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EVENT,
		"<gContact:event xmlns:gContact='http://schemas.google.com/contact/2008' xmlns:gd='http://schemas.google.com/g/2005' "
		  "label='&lt;a&gt;'>"
			"<gd:when startTime='2000-01-01'/>"
		"</gContact:event>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_EVENT (event));
	g_clear_error (&error);

	/* Check the properties */
	gdata_gcontact_event_get_date (event, &date);
	g_assert (g_date_valid (&date) == TRUE);
	g_assert_cmpuint (g_date_get_year (&date), ==, 2000);
	g_assert_cmpuint (g_date_get_month (&date), ==, 1);
	g_assert_cmpuint (g_date_get_day (&date), ==, 1);
	g_assert (gdata_gcontact_event_get_relation_type (event) == NULL);
	g_assert_cmpstr (gdata_gcontact_event_get_label (event), ==, "<a>");

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (event));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:event xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005' "
			   "xmlns:gContact='http://schemas.google.com/contact/2008' label='&lt;a&gt;'>"
				"<gd:when startTime='2000-01-01'/>"
			 "</gContact:event>");
	g_free (xml);
	g_object_unref (event);
}

static void
test_gcontact_event_error_handling (void)
{
	GDataGContactEvent *event;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) event = GDATA_GCONTACT_EVENT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EVENT,\
		"<gContact:event xmlns:gContact='http://schemas.google.com/contact/2008' xmlns:gd='http://schemas.google.com/g/2005' "\
			x\
		"</gContact:event>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (event == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING ("rel='anniversary'>"); /* no gd:when */
	TEST_XML_ERROR_HANDLING ("rel='anniversary'><gd:when/>"); /* no startTime */
	TEST_XML_ERROR_HANDLING ("rel='anniversary'><gd:when startTime='foobar'/>"); /* invalid startTime */
	TEST_XML_ERROR_HANDLING ("rel='anniversary'><gd:when startTime='2001-12-41'/>");
	TEST_XML_ERROR_HANDLING ("rel='anniversary'><gd:when startTime='2010-03-25T22:01Z'/>");
	TEST_XML_ERROR_HANDLING ("><gd:when startTime='2000-01-01'/>"); /* no rel or label */
	TEST_XML_ERROR_HANDLING ("rel=''><gd:when startTime='2000-01-01'/>"); /* empty rel */
	TEST_XML_ERROR_HANDLING ("label=''><gd:when startTime='2000-01-01'/>"); /* empty label */
	TEST_XML_ERROR_HANDLING ("rel='other' label='Other'><gd:when startTime='2000-01-01'/>"); /* rel and label */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_external_id (void)
{
	GDataGContactExternalID *id, *id2;
	gchar *xml;
	GError *error = NULL;

	id = GDATA_GCONTACT_EXTERNAL_ID (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EXTERNAL_ID,
		"<gContact:externalId xmlns:gContact='http://schemas.google.com/contact/2008' rel='account' value='5'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_EXTERNAL_ID (id));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_external_id_get_value (id), ==, "5");
	g_assert_cmpstr (gdata_gcontact_external_id_get_relation_type (id), ==, GDATA_GCONTACT_EXTERNAL_ID_ACCOUNT);
	g_assert (gdata_gcontact_external_id_get_label (id) == NULL);

	/* Compare it against another identical external ID */
	id2 = gdata_gcontact_external_id_new ("5", GDATA_GCONTACT_EXTERNAL_ID_ACCOUNT, NULL);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (id), GDATA_COMPARABLE (id2)), ==, 0);

	/* …and a different one */
	gdata_gcontact_external_id_set_value (id2, "http://identifying.uri");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (id), GDATA_COMPARABLE (id2)), !=, 0);
	g_object_unref (id2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (id));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:externalId xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"value='5' rel='account'/>");
	g_free (xml);
	g_object_unref (id);

	/* Now parse an ID with less information available */
	id = GDATA_GCONTACT_EXTERNAL_ID (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EXTERNAL_ID,
		"<gContact:externalId xmlns:gContact='http://schemas.google.com/contact/2008' value='' label='&lt;a&gt;'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_EXTERNAL_ID (id));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_external_id_get_value (id), ==, "");
	g_assert (gdata_gcontact_external_id_get_relation_type (id) == NULL);
	g_assert_cmpstr (gdata_gcontact_external_id_get_label (id), ==, "<a>");

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (id));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:externalId xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"value='' label='&lt;a&gt;'/>");
	g_free (xml);
	g_object_unref (id);
}

static void
test_gcontact_external_id_error_handling (void)
{
	GDataGContactExternalID *id;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) id = GDATA_GCONTACT_EXTERNAL_ID (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_EXTERNAL_ID,\
		"<gContact:externalId xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"/>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (id == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING ("rel='account'"); /* no value */
	TEST_XML_ERROR_HANDLING ("value='foo'"); /* no rel or label */
	TEST_XML_ERROR_HANDLING ("value='foo' rel=''"); /* empty rel */
	TEST_XML_ERROR_HANDLING ("value='foo' label=''"); /* empty label */
	TEST_XML_ERROR_HANDLING ("value='foo' rel='organization' label='Other'"); /* rel and label */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_jot (void)
{
	GDataGContactJot *jot, *jot2;
	gchar *xml;
	GError *error = NULL;

	jot = GDATA_GCONTACT_JOT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_JOT,
		"<gContact:jot xmlns:gContact='http://schemas.google.com/contact/2008' rel='user'>They like &lt;angles&gt;.</gContact:jot>",
		-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_JOT (jot));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_jot_get_relation_type (jot), ==, GDATA_GCONTACT_JOT_USER);
	g_assert_cmpstr (gdata_gcontact_jot_get_content (jot), ==, "They like <angles>.");

	/* Try creating another jot from scratch */
	jot2 = gdata_gcontact_jot_new ("friend,local,oss", GDATA_GCONTACT_JOT_KEYWORDS);
	g_assert (GDATA_IS_GCONTACT_JOT (jot2));
	g_object_unref (jot2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (jot));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:jot xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' rel='user'>"
				"They like &lt;angles&gt;.</gContact:jot>");
	g_free (xml);
	g_object_unref (jot);

	/* Now parse a jot with different information available */
	jot = GDATA_GCONTACT_JOT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_JOT,
		"<gContact:jot xmlns:gContact='http://schemas.google.com/contact/2008' rel='other'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_JOT (jot));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_jot_get_relation_type (jot), ==, GDATA_GCONTACT_JOT_OTHER);
	g_assert (gdata_gcontact_jot_get_content (jot) == NULL);

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (jot));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:jot xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' rel='other'/>");
	g_free (xml);
	g_object_unref (jot);
}

static void
test_gcontact_jot_error_handling (void)
{
	GDataGContactJot *jot;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) jot = GDATA_GCONTACT_JOT (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_JOT,\
		"<gContact:jot xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"</gContact:jot>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (jot == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING (">Content"); /* no rel */
	TEST_XML_ERROR_HANDLING ("rel=''>Content"); /* empty rel */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_language (void)
{
	GDataGContactLanguage *language, *language2;
	gchar *xml;
	GError *error = NULL;

	language = GDATA_GCONTACT_LANGUAGE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_LANGUAGE,
		"<gContact:language xmlns:gContact='http://schemas.google.com/contact/2008' code='en-GB'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_LANGUAGE (language));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_language_get_code (language), ==, "en-GB");
	g_assert (gdata_gcontact_language_get_label (language) == NULL);

	/* Compare it against another identical language */
	language2 = gdata_gcontact_language_new ("en-GB", NULL);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (language), GDATA_COMPARABLE (language2)), ==, 0);

	/* …and a different one */
	gdata_gcontact_language_set_code (language2, "sv");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (language), GDATA_COMPARABLE (language2)), !=, 0);
	g_object_unref (language2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (language));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:language xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"code='en-GB'/>");
	g_free (xml);
	g_object_unref (language);

	/* Now parse a language with less information available */
	language = GDATA_GCONTACT_LANGUAGE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_LANGUAGE,
		"<gContact:language xmlns:gContact='http://schemas.google.com/contact/2008' label='Gobbledegook'/>",
		-1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_LANGUAGE (language));
	g_clear_error (&error);

	/* Check the properties */
	g_assert (gdata_gcontact_language_get_code (language) == NULL);
	g_assert_cmpstr (gdata_gcontact_language_get_label (language), ==, "Gobbledegook");

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (language));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:language xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"label='Gobbledegook'/>");
	g_free (xml);
	g_object_unref (language);
}

static void
test_gcontact_language_error_handling (void)
{
	GDataGContactLanguage *language;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) language = GDATA_GCONTACT_LANGUAGE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_LANGUAGE,\
		"<gContact:language xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"/>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (language == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING (""); /* no code or label */
	TEST_XML_ERROR_HANDLING ("code=''"); /* empty code */
	TEST_XML_ERROR_HANDLING ("label=''"); /* empty label */
	TEST_XML_ERROR_HANDLING ("code='en-GB' label='Other'"); /* code and label */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_relation (void)
{
	GDataGContactRelation *relation, *relation2;
	gchar *xml;
	GError *error = NULL;

	relation = GDATA_GCONTACT_RELATION (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_RELATION,
		"<gContact:relation xmlns:gContact='http://schemas.google.com/contact/2008' rel='child'>Fred</gContact:relation>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_RELATION (relation));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_relation_get_name (relation), ==, "Fred");
	g_assert_cmpstr (gdata_gcontact_relation_get_relation_type (relation), ==, GDATA_GCONTACT_RELATION_CHILD);
	g_assert (gdata_gcontact_relation_get_label (relation) == NULL);

	/* Try creating another relation from scratch */
	relation2 = gdata_gcontact_relation_new ("Brian", GDATA_GCONTACT_RELATION_RELATIVE, NULL);
	g_assert (GDATA_IS_GCONTACT_RELATION (relation2));
	g_object_unref (relation2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (relation));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:relation xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' rel='child'>"
				"Fred</gContact:relation>");
	g_free (xml);
	g_object_unref (relation);

	/* Now parse a relation with different information available */
	relation = GDATA_GCONTACT_RELATION (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_RELATION,
		"<gContact:relation xmlns:gContact='http://schemas.google.com/contact/2008' label='&lt;a&gt;'>Sid</gContact:relation>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_RELATION (relation));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_relation_get_name (relation), ==, "Sid");
	g_assert (gdata_gcontact_relation_get_relation_type (relation) == NULL);
	g_assert_cmpstr (gdata_gcontact_relation_get_label (relation), ==, "<a>");

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (relation));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:relation xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
			   "label='&lt;a&gt;'>Sid</gContact:relation>");
	g_free (xml);
	g_object_unref (relation);
}

static void
test_gcontact_relation_error_handling (void)
{
	GDataGContactRelation *relation;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) relation = GDATA_GCONTACT_RELATION (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_RELATION,\
		"<gContact:relation xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"</gContact:relation>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (relation == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING ("rel='spouse'>"); /* no name */
	TEST_XML_ERROR_HANDLING (">Brian"); /* no rel or label */
	TEST_XML_ERROR_HANDLING ("rel=''>Brian"); /* empty rel */
	TEST_XML_ERROR_HANDLING ("label=''>Brian"); /* empty label */
	TEST_XML_ERROR_HANDLING ("rel='sister' label='Older sister'>Brian"); /* rel and label */

#undef TEST_XML_ERROR_HANDLING
}

static void
test_gcontact_website (void)
{
	GDataGContactWebsite *website, *website2;
	gchar *xml;
	GError *error = NULL;

	website = GDATA_GCONTACT_WEBSITE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_WEBSITE,
		"<gContact:website xmlns:gContact='http://schemas.google.com/contact/2008' href='http://example.com/' rel='work' primary='true' "
			"label='&lt;Markup&gt; blog'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_WEBSITE (website));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_website_get_uri (website), ==, "http://example.com/");
	g_assert_cmpstr (gdata_gcontact_website_get_relation_type (website), ==, GDATA_GCONTACT_WEBSITE_WORK);
	g_assert_cmpstr (gdata_gcontact_website_get_label (website), ==, "<Markup> blog");
	g_assert (gdata_gcontact_website_is_primary (website) == TRUE);

	/* Compare it against another identical website */
	website2 = gdata_gcontact_website_new ("http://example.com/", GDATA_GCONTACT_WEBSITE_WORK, "<Markup> blog", TRUE);
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (website), GDATA_COMPARABLE (website2)), ==, 0);

	/* …and a different one */
	gdata_gcontact_website_set_uri (website2, "http://somewhereelse.com/");
	g_assert_cmpint (gdata_comparable_compare (GDATA_COMPARABLE (website), GDATA_COMPARABLE (website2)), !=, 0);
	g_object_unref (website2);

	/* Check the outputted XML is the same */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (website));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:website xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"href='http://example.com/' rel='work' label='&lt;Markup&gt; blog' primary='true'/>");
	g_free (xml);
	g_object_unref (website);

	/* Now parse a website with less information available */
	website = GDATA_GCONTACT_WEBSITE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_WEBSITE,
		"<gContact:website xmlns:gContact='http://schemas.google.com/contact/2008' href='http://test.com/' rel='ftp'/>", -1, &error));
	g_assert_no_error (error);
	g_assert (GDATA_IS_GCONTACT_WEBSITE (website));
	g_clear_error (&error);

	/* Check the properties */
	g_assert_cmpstr (gdata_gcontact_website_get_uri (website), ==, "http://test.com/");
	g_assert_cmpstr (gdata_gcontact_website_get_relation_type (website), ==, GDATA_GCONTACT_WEBSITE_FTP);
	g_assert (gdata_gcontact_website_get_label (website) == NULL);
	g_assert (gdata_gcontact_website_is_primary (website) == FALSE);

	/* Check the outputted XML is still OK */
	xml = gdata_parsable_get_xml (GDATA_PARSABLE (website));
	g_assert_cmpstr (xml, ==,
			 "<?xml version='1.0' encoding='UTF-8'?>"
			 "<gContact:website xmlns='http://www.w3.org/2005/Atom' xmlns:gContact='http://schemas.google.com/contact/2008' "
				"href='http://test.com/' rel='ftp' primary='false'/>");
	g_free (xml);
	g_object_unref (website);
}

static void
test_gcontact_website_error_handling (void)
{
	GDataGContactWebsite *website;
	GError *error = NULL;

#define TEST_XML_ERROR_HANDLING(x) website = GDATA_GCONTACT_WEBSITE (gdata_parsable_new_from_xml (GDATA_TYPE_GCONTACT_WEBSITE,\
		"<gContact:website xmlns:gContact='http://schemas.google.com/contact/2008' "\
			x\
		"/>", -1, &error));\
	g_assert_error (error, GDATA_SERVICE_ERROR, GDATA_SERVICE_ERROR_PROTOCOL_ERROR);\
	g_assert (website == NULL);\
	g_clear_error (&error)

	TEST_XML_ERROR_HANDLING ("rel='work'"); /* no href */
	TEST_XML_ERROR_HANDLING ("rel='work' href=''"); /* empty href */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/'"); /* no rel */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel=''"); /* empty rel */
	TEST_XML_ERROR_HANDLING ("href='http://example.com/' rel='profile' primary='not a boolean'"); /* invalid primary */

#undef TEST_XML_ERROR_HANDLING
}

int
main (int argc, char *argv[])
{
	gdata_test_init (argc, argv);

	g_test_add_func ("/service/network_error", test_service_network_error);
	g_test_add_func ("/service/locale", test_service_locale);

	g_test_add_func ("/entry/get_xml", test_entry_get_xml);
	g_test_add_func ("/entry/parse_xml", test_entry_parse_xml);
	g_test_add_func ("/entry/error_handling", test_entry_error_handling);

	g_test_add_func ("/feed/parse_xml", test_feed_parse_xml);
	g_test_add_func ("/feed/error_handling", test_feed_error_handling);

	g_test_add_func ("/query/categories", test_query_categories);
	g_test_add_func ("/query/unicode", test_query_unicode);
	g_test_add_func ("/query/etag", test_query_etag);

	g_test_add_func ("/access-rule/get_xml", test_access_rule_get_xml);
	g_test_add_func ("/access-rule/error_handling", test_access_rule_error_handling);

	g_test_add_func ("/comparable", test_comparable);

	g_test_add_func ("/color/parsing", test_color_parsing);
	g_test_add_func ("/color/output", test_color_output);

	g_test_add_func ("/atom/author", test_atom_author);
	g_test_add_func ("/atom/author/error_handling", test_atom_author_error_handling);
	g_test_add_func ("/atom/category", test_atom_category);
	g_test_add_func ("/atom/category/error_handling", test_atom_category_error_handling);
	g_test_add_func ("/atom/generator", test_atom_generator);
	g_test_add_func ("/atom/generator/error_handling", test_atom_generator_error_handling);
	g_test_add_func ("/atom/link", test_atom_link);
	g_test_add_func ("/atom/link/error_handling", test_atom_link_error_handling);

	g_test_add_func ("/app/categories", test_app_categories);

	g_test_add_func ("/gd/email_address", test_gd_email_address);
	g_test_add_func ("/gd/email_address/escaping", test_gd_email_address_escaping);
	g_test_add_func ("/gd/im_address", test_gd_im_address);
	g_test_add_func ("/gd/im_address/escaping", test_gd_im_address_escaping);
	g_test_add_func ("/gd/name", test_gd_name);
	g_test_add_func ("/gd/organization", test_gd_organization);
	g_test_add_func ("/gd/organization/escaping", test_gd_organization_escaping);
	g_test_add_func ("/gd/phone_number", test_gd_phone_number);
	g_test_add_func ("/gd/phone_number/escaping", test_gd_phone_number_escaping);
	g_test_add_func ("/gd/postal_address", test_gd_postal_address);
	g_test_add_func ("/gd/postal_address/escaping", test_gd_postal_address_escaping);
	g_test_add_func ("/gd/reminder", test_gd_reminder);
	g_test_add_func ("/gd/reminder/escaping", test_gd_reminder_escaping);
	g_test_add_func ("/gd/when", test_gd_when);
	g_test_add_func ("/gd/when/escaping", test_gd_when_escaping);
	g_test_add_func ("/gd/where", test_gd_where);
	g_test_add_func ("/gd/where/escaping", test_gd_where_escaping);
	g_test_add_func ("/gd/who", test_gd_who);
	g_test_add_func ("/gd/who/escaping", test_gd_who_escaping);

	g_test_add_func ("/media/category", test_media_category);
	g_test_add_func ("/media/content", test_media_content);
	g_test_add_func ("/media/credit", test_media_credit);
	/* g_test_add_func ("/media/group", test_media_group); */
	g_test_add_func ("/media/thumbnail", test_media_thumbnail);
	/*g_test_add_data_func ("/media/thumbnail/parse_time", "", test_media_thumbnail_parse_time);
	g_test_add_data_func ("/media/thumbnail/parse_time", "de_DE", test_media_thumbnail_parse_time);*/

	g_test_add_func ("/gcontact/calendar", test_gcontact_calendar);
	g_test_add_func ("/gcontact/calendar/error_handling", test_gcontact_calendar_error_handling);
	g_test_add_func ("/gcontact/event", test_gcontact_event);
	g_test_add_func ("/gcontact/event/error_handling", test_gcontact_event_error_handling);
	g_test_add_func ("/gcontact/external_id", test_gcontact_external_id);
	g_test_add_func ("/gcontact/external_id/error_handling", test_gcontact_external_id_error_handling);
	g_test_add_func ("/gcontact/jot", test_gcontact_jot);
	g_test_add_func ("/gcontact/jot/error_handling", test_gcontact_jot_error_handling);
	g_test_add_func ("/gcontact/language", test_gcontact_language);
	g_test_add_func ("/gcontact/language/error_handling", test_gcontact_language_error_handling);
	g_test_add_func ("/gcontact/relation", test_gcontact_relation);
	g_test_add_func ("/gcontact/relation/error_handling", test_gcontact_relation_error_handling);
	g_test_add_func ("/gcontact/website", test_gcontact_website);
	g_test_add_func ("/gcontact/website/error_handling", test_gcontact_website_error_handling);

	return g_test_run ();
}
