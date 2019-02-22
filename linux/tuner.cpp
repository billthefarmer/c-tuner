////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A tuner written in C++.
//
//  Copyright (C) 2019  Bill Farmer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include "tuner.h"

// Main function
int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    // Restore options
    restoreOptions();

    app = gtk_application_new(APP_ID, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Stop audio
    audio.done = true;
    snd_pcm_close(audio.handle);
    saveOptions();

    return status;
}

// Application
void activate(GtkApplication *app, gpointer data)
{
    // Widgets
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *quit;
    GtkWidget *options;
    
    GdkGeometry geometry =
        {
            -1, -1, -1, -1, -1, -1, 1, 1,
            380.0/510.0, 380.0/510.0
        };

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Tuner");
    gtk_window_set_resizable(GTK_WINDOW(window), true);
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &geometry,
                                  GDK_HINT_ASPECT);
    // V box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, MARGIN);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, true, true, MARGIN);

    // V box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, MARGIN);

    // Scope
    scope.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(scope.widget, DISPLAY_WIDTH, SCOPE_HEIGHT);
    gtk_widget_set_name(scope.widget, "scope");
    gtk_box_pack_start(GTK_BOX(vbox), scope.widget, true, true, 0);

    g_signal_connect(G_OBJECT(scope.widget), "draw",
		     G_CALLBACK(scope_draw_callback), NULL);

    // Button pressed callback
    g_signal_connect(G_OBJECT(scope.widget), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(scope.widget, GDK_BUTTON_PRESS_MASK);

    // Spectrum
    spectrum.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(spectrum.widget,
				DISPLAY_WIDTH, SPECTRUM_HEIGHT);
    gtk_widget_set_name(spectrum.widget, "spectrum");
    gtk_box_pack_start(GTK_BOX(vbox), spectrum.widget, true, true, 0);

    g_signal_connect(G_OBJECT(spectrum.widget), "draw",
		     G_CALLBACK(spectrum_draw_callback), NULL);

    // Button pressed callback
    g_signal_connect(G_OBJECT(spectrum.widget), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(spectrum.widget, GDK_BUTTON_PRESS_MASK);

    // Display
    display.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(display.widget, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    gtk_widget_set_name(display.widget, "display");
    gtk_box_pack_start(GTK_BOX(vbox), display.widget, true, true, 0);

    g_signal_connect(G_OBJECT(display.widget), "draw",
		     G_CALLBACK(display_draw_callback), NULL);

    // Button pressed callback
    g_signal_connect(G_OBJECT(display.widget), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(display.widget, GDK_BUTTON_PRESS_MASK);

    // Strobe
    strobe.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(strobe.widget, DISPLAY_WIDTH, STROBE_HEIGHT);
    gtk_widget_set_name(strobe.widget, "strobe");
    gtk_box_pack_start(GTK_BOX(vbox), strobe.widget, true, true, 0);

    g_signal_connect(G_OBJECT(strobe.widget), "draw",
		     G_CALLBACK(strobe_draw_callback), NULL);

    // Button pressed callback
    g_signal_connect(G_OBJECT(strobe.widget), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(strobe.widget, GDK_BUTTON_PRESS_MASK);

    // Meter
    meter.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(meter.widget, DISPLAY_WIDTH, METER_HEIGHT);
    gtk_widget_set_name(meter.widget, "meter");
    gtk_box_pack_start(GTK_BOX(vbox), meter.widget, true, true, 0);

    g_signal_connect(G_OBJECT(meter.widget), "draw",
		     G_CALLBACK(meter_draw_callback), NULL);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, false, false, 0);

    // Options button
    options = gtk_button_new_with_label(" Options\u2026 ");
    gtk_box_pack_start(GTK_BOX(hbox), options, false, false, 0);

    // Options clicked
    g_signal_connect(G_OBJECT(options), "clicked",
		     G_CALLBACK(options_clicked), window);

    // Quit button
    quit = gtk_button_new_with_label("    Quit    ");
    gtk_box_pack_end(GTK_BOX(hbox), quit, false, false, 0);

    // Quit clicked
    g_signal_connect_swapped(G_OBJECT(quit), "clicked",
			     G_CALLBACK(gtk_widget_destroy), window);

    // Key pressed callback
    g_signal_connect(G_OBJECT(window), "key-press-event",
		     G_CALLBACK(key_press), NULL);

    // Button pressed callback
    g_signal_connect(G_OBJECT(window), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

    // Show the window
    gtk_widget_show_all(window);

    // Start audio
    initAudio();
}

// Restore options
void restoreOptions()
{
    // Initial values
    spectrum.zoom = true;
    spectrum.expand = 1;

    audio.reference = A5_REFERENCE;
    audio.correction = 1.0;

    // Get user home
    char *home = getenv("HOME");
    char name[64];

    // Build options path
    strcpy(name, home);
    strcat(name, "/.tuner.ini");

    // Open options file
    FILE *file = fopen(name, "r");

    if (file == NULL)
	return;

    // Scan the file
    while (!feof(file))
    {
	char line[64];
	gboolean found = false;

	if (fgets(line, sizeof(line), file) == 0)
	    continue;

	// Look for [Options]
	if (strncmp(line, "[Options]", strlen("[Options]")) == 0)
	{
	    found = true;
	    continue;
	}

	// Look for option values
	if (found)
	{
	    if (strncmp(line, "filter", strlen("filter")) == 0)
	    {
		sscanf(line, "filter=%d", &audio.filter);
		continue;
	    }

	    if (strncmp(line, "downsample", strlen("downsample")) == 0)
	    {
		sscanf(line, "downsample=%d", &audio.downsample);
		continue;
	    }

	    if (strncmp(line, "lock", strlen("lock")) == 0)
	    {
		sscanf(line, "lock=%d", &display.lock);
		continue;
	    }

	    if (strncmp(line, "multiple", strlen("multiple")) == 0)
	    {
		sscanf(line, "multiple=%d", &display.multiple);
		continue;
	    }

	    if (strncmp(line, "strobe", strlen("strobe")) == 0)
	    {
		sscanf(line, "strobe=%d", &strobe.enable);
		continue;
	    }

	    if (strncmp(line, "colours", strlen("colours")) == 0)
	    {
		sscanf(line, "colours=%d", &strobe.colours);
		continue;
	    }

	    if (strncmp(line, "zoom", strlen("zoom")) == 0)
	    {
		sscanf(line, "zoom=%d", &spectrum.zoom);
		continue;
	    }

	    if (strncmp(line, "reference", strlen("reference")) == 0)
	    {
		sscanf(line, "reference=%lf", &audio.reference);
		continue;
	    }

	    if (strncmp(line, "correction", strlen("correction")) == 0)
	    {
		sscanf(line, "correction=%lf", &audio.correction);

		if (audio.correction != 1.0)
		    audio.save = true;

		continue;
	    }
	}
    }

    fclose(file);
}

// Save options
void saveOptions()
{
    char *home = getenv("HOME");
    char name[64];

    strcpy(name, home);
    strcat(name, "/.tuner.ini");

    FILE *file = fopen(name, "w");

    fputs("# Tuner options\n#\n[Options]\n", file);
    fprintf(file, "filter=%d\n", audio.filter);
    fprintf(file, "downsample=%d\n", audio.downsample);
    fprintf(file, "lock=%d\n", display.lock);
    fprintf(file, "multiple=%d\n", display.multiple);
    fprintf(file, "strobe=%d\n", strobe.enable);
    fprintf(file, "colours=%d\n", strobe.colours);
    fprintf(file, "zoom=%d\n", spectrum.zoom);
    fprintf(file, "reference=%1.2f\n", audio.reference);

    if (audio.save)
	fprintf(file, "correction=%1.6f\n", audio.correction);

    fclose(file);
}

// Init audio
void initAudio(void)
{
    int err, dir;
    unsigned rate_min, rate_max;
    snd_pcm_hw_params_t *hwparams;

    if ((err = snd_pcm_open(&audio.handle, "default",
			    SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
	printf("Capture open error: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_malloc(&hwparams)) < 0)
    {
	printf("Capture allocate error: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_any(audio.handle, hwparams)) < 0)
    {
	printf("Setting of hwparams failed: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_get_rate_min(hwparams, &rate_min, &dir)) < 0)
    {
	printf("Getting of min rate failed: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_get_rate_max(hwparams, &rate_max, &dir)) < 0)
    {
	printf("Getting of max rate failed: %s\n", snd_strerror(err));
	return;
    }

    if ((SAMPLE_RATE < rate_min) || (SAMPLE_RATE > rate_max))
    {
	printf("Required sample rate %d outside available range %d - %d\n",
	       SAMPLE_RATE, rate_min, rate_max);
	return;
    }

    if ((err = snd_pcm_set_params(audio.handle, SND_PCM_FORMAT_S16,
				  SND_PCM_ACCESS_RW_INTERLEAVED,
				  CHANNELS, SAMPLE_RATE,
				  1, LATENCY)) < 0)
    {
	printf("Setting parameters failed: %s\n", snd_strerror(err));
	return;
    }

    // Create the audio thread
    pthread_create(&audio.thread, NULL, readAudio, NULL);

}

// Read audio
void *readAudio(void *)
{
    enum
    {TIMER_COUNT = 16};

    int err;
    snd_pcm_sframes_t frames = 0;

    // Create buffers for processing the audio data
    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double x2[RANGE / 2];
    static double x3[RANGE / 3];
    static double x4[RANGE / 4];
    static double x5[RANGE / 5];

    static double dx[RANGE];

    static maximum maxima[MAXIMA];
    static value   values[MAXIMA];

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

    static short data[STEP];

    // Initialise data structs
    if (scope.data == NULL)
    {
	scope.data = data;
	scope.length = STEP;

	spectrum.data = xa;
	spectrum.length = RANGE;

	spectrum.values = values;
	display.maxima = maxima;
    }

    while (!audio.done)
    {
	if ((frames = snd_pcm_readi(audio.handle, data, STEP)) < 0)
	    break;

	// Copy the input data
	memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(double));

	// Butterworth filter, 3dB/octave
	for (int i = 0; i < STEP; i++)
	{
	    static double G = 3.023332184e+01;
	    static double K = 0.9338478249;

	    static double xv[2];
	    static double yv[2];

	    xv[0] = xv[1];
	    xv[1] = (double)data[i] / G;

	    yv[0] = yv[1];
	    yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	    // Choose filtered/unfiltered data
	    buffer[(SAMPLES - STEP) + i] =
		audio.filter? yv[1]: (double)data[i];
	}

	// Maximum data value
	static double dmax;

	if (dmax < 4096.0)
	    dmax = 4096.0;

	// Calculate normalising value
	double norm = dmax;
	dmax = 0.0;

	// Copy data to FFT input arrays for tuner
	for (int i = 0; i < SAMPLES; i++)
	{
	    // Find the magnitude
	    if (dmax < fabs(buffer[i]))
		dmax = fabs(buffer[i]);

	    // Calculate the window
	    double window =
		0.5 - 0.5 * cos(2.0 * M_PI *
				i / SAMPLES);

	    // Normalise and window the input data
	    x[i].r = (double)buffer[i] / norm * window;
	}

	// do FFT for tuner
	fftr(x, SAMPLES);

	// Process FFT output for tuner
	for (int i = 1; i < RANGE; i++)
	{
	    double real = x[i].r;
	    double imag = x[i].i;

	    xa[i] = hypot(real, imag);

	    // Do frequency calculation
	    double p = atan2(imag, real);
	    double dp = xp[i] - p;
	    xp[i] = p;

	    // Calculate phase difference
	    dp -= (double)i * expect;

	    int qpd = dp / M_PI;

	    if (qpd >= 0)
		qpd += qpd & 1;

	    else
		qpd -= qpd & 1;

	    dp -=  M_PI * (double)qpd;

	    // Calculate frequency difference
	    double df = OVERSAMPLE * dp / (2.0 * M_PI);

	    // Calculate actual frequency from slot frequency plus
	    // frequency difference and correction value
	    xf[i] = (i * fps + df * fps) / audio.correction;

	    // Calculate differences for finding maxima
	    dx[i] = xa[i] - xa[i - 1];
	}

	// Downsample
	if (audio.downsample)
	{
	    // x2 = xa << 2
	    for (unsigned int i = 0; i < Length(x2); i++)
	    {
		x2[i] = 0.0;

		for (int j = 0; j < 2; j++)
		    x2[i] += xa[(i * 2) + j] / 2.0;
	    }

	    // x3 = xa << 3
	    for (unsigned int i = 0; i < Length(x3); i++)
	    {
		x3[i] = 0.0;

		for (int j = 0; j < 3; j++)
		    x3[i] += xa[(i * 3) + j] / 3.0;
	    }

	    // x4 = xa << 4
	    for (unsigned int i = 0; i < Length(x4); i++)
	    {
		x4[i] = 0.0;

		for (int j = 0; j < 4; j++)
		    x2[i] += xa[(i * 4) + j] / 4.0;
	    }

	    // x5 = xa << 5
	    for (unsigned int i = 0; i < Length(x5); i++)
	    {
		x5[i] = 0.0;

		for (int j = 0; j < 5; j++)
		    x5[i] += xa[(i * 5) + j] / 5.0;
	    }

	    // Add downsamples
	    for (unsigned int i = 1; i < Length(xa); i++)
	    {
		if (i < Length(x2))
		    xa[i] += x2[i];

		if (i < Length(x3))
		    xa[i] += x3[i];

		if (i < Length(x4))
		    xa[i] += x4[i];

		if (i < Length(x5))
		    xa[i] += x5[i];

		// Recalculate differences
		dx[i] = xa[i] - xa[i - 1];
	    }
	}

	// Maximum FFT output
	double max = 0.0;
	double f = 0.0;

	unsigned int count = 0;
	int limit = RANGE - 1;

	// Find maximum value, and list of maxima
	for (int i = 1; i < limit; i++)
	{
	    if (xa[i] > max)
	    {
		max = xa[i];
		f = xf[i];
	    }

	    // If display not locked, find maxima and add to list
	    if (!display.lock && count < Length(maxima) &&
		xa[i] > MINIMUM && xa[i] > (max / 4.0) &&
		dx[i] > 0.0 && dx[i + 1] < 0.0)
	    {
		maxima[count].f = xf[i];

		// Cents relative to reference
		double cf =
		    -12.0 * log2(audio.reference / xf[i]);

		// Reference note
		maxima[count].fr = audio.reference * pow(2.0, round(cf) / 12.0);

		// Note number
		maxima[count].n = round(cf) + C5_OFFSET;

		// Set limit to octave above
		if (!audio.downsample && (limit > i * 2))
		    limit = i * 2 - 1;

		count++;
	    }
	}

	// Reference note frequency and lower and upper limits
	double fr = 0.0;
	double fl = 0.0;
	double fh = 0.0;

	// Note number
	int n = 0;

	// Found flag and cents value
	gboolean found = false;
	double c = 0.0;

	// Do the note and cents calculations
	if (max > MINIMUM)
	{
	    found = true;

	    // Frequency
	    if (!audio.downsample)
		f = maxima[0].f;

	    // Cents relative to reference

	    double cf =
		-12.0 * log2(audio.reference / f);

	    // Reference note
	    fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    // Lower and upper freq
	    fl = audio.reference * pow(2.0, (round(cf) - 0.55) / 12.0);
	    fh = audio.reference * pow(2.0, (round(cf) + 0.55) / 12.0);

	    // Note number
	    n = round(cf) + C5_OFFSET;

	    if (n < 0)
		found = false;

	    // Find nearest maximum to reference note
	    double df = 1000.0;

	    for (unsigned int i = 0; i < count; i++)
	    {
		if (fabs(maxima[i].f - fr) < df)
		{
		    df = fabs(maxima[i].f - fr);
		    f = maxima[i].f;
		}
	    }

	    // Cents relative to reference note
	    c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		c = 0.0;

	    // Ignore if not within 50 cents of reference note
	    if (fabs(c) > 0.5)
		found = false;
	}

	// If display not locked
	if (!display.lock)
	{
	    // Update scope window
	    widget_queue_draw(scope.widget);

	    // Update spectrum window
	    for (unsigned int i = 0; i < count; i++)
		values[i].f = maxima[i].f / fps * audio.correction;

	    spectrum.count = count;

	    if (found)
	    {
		spectrum.f = f  / fps * audio.correction;
		spectrum.r = fr / fps * audio.correction;
		spectrum.l = fl / fps * audio.correction;
		spectrum.h = fh / fps * audio.correction;
	    }

	    // Update spectrum
	    widget_queue_draw(spectrum.widget);
	}

	static long timer;

	if (found)
	{
	    // If display not locked
	    if (!display.lock)
	    {
		// Update the display struct
		display.f = f;
		display.fr = fr;
		display.c = c;
		display.n = n;
		display.count = count;

		// Update meter
		meter.c = c;

		// Update strobe
		strobe.c = c;
	    }

	    // Update display
	    widget_queue_draw(display.widget);

	    // Reset count;
	    timer = 0;
	}

	else
	{
	    // If display not locked
	    if (!display.lock)
	    {

		if (timer > TIMER_COUNT)
		{
		    display.f = 0.0;
		    display.fr = 0.0;
		    display.c = 0.0;
		    display.n = 0;
		    display.count = 0;

		    // Update meter
		    meter.c = 0.0;

		    // Update strobe
		    strobe.c = 0.0;

		    // Update spectrum
		    spectrum.f = 0.0;
		    spectrum.r = 0.0;
		    spectrum.l = 0.0;
		    spectrum.h = 0.0;
		}

		// Update display
		widget_queue_draw(display.widget);
	    }
	}

	widget_queue_draw(strobe.widget);
	widget_queue_draw(meter.widget);

	timer++;
    }

    if (frames < 0)
	printf("Read error: %s\n", snd_strerror(frames));

    if ((err = snd_pcm_close(audio.handle)) < 0)
    {
	printf("Capture close error: %s\n", snd_strerror(err));
	return NULL;
    }

    return NULL;
}

// Real to complex FFT, ignores imaginary values in input array
void fftr(complex a[], int n)
{
    double norm = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].r * norm;

	    a[j].r = a[i].r * norm;
	    a[j].i = 0.0;

	    a[i].r = tr;
	    a[i].i = 0.0;
	}

	int m = n / 2;
	while (m >= 1 && j >= m)
	{
	    j -= m;
	    m /= 2;
	}
	j += m;
    }
    
    for (int mmax = 1, istep = 2 * mmax; mmax < n;
	 mmax = istep, istep = 2 * mmax)
    {
	double delta = (M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].r - wi * a[j].i;
		double ti = wr * a[j].i + wi * a[j].r;
		a[j].r = a[i].r - tr;
		a[j].i = a[i].i - ti;
		a[i].r += tr;
		a[i].i += ti;
	    }
	}
    }
}

// Draw widget
void widget_queue_draw(gpointer widget)
{
    // Must have at least one lambda expression
    gdk_threads_add_idle([](gpointer widget) -> gboolean
    {
        gtk_widget_queue_draw(GTK_WIDGET(widget));
        return G_SOURCE_REMOVE;
    }, widget);
}

// Round rect
void cairo_round_rect(cairo_t *cr, double x, double y,
		      double w, double h, double r)
{
    cairo_move_to(cr, x + r, y);
    cairo_line_to(cr, x + w - r, y);
    cairo_arc(cr, x + w - r, y + r, r, -M_PI / 2, 0);
    cairo_line_to(cr, x + w, y + h - r);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_line_to(cr, x + r, y + h);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_line_to(cr, x, y + r);
    cairo_arc(cr, x + r, y + r, r, M_PI, M_PI * 3 / 2);
}

// Draw edge
void cairo_edge(cairo_t *cr, int w, int h)
{
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 3);
    cairo_round_rect(cr, 0, 0, w, h, 5);
    cairo_stroke(cr);

    cairo_round_rect(cr, 2, 2, w - 4, h - 4, 5);
    cairo_clip(cr);

    cairo_translate(cr, 2, 2);
}

// Right justify text
void cairo_right_justify_text(cairo_t *cr, char *t)
{
    double x, y;
    cairo_text_extents_t extents;

    cairo_get_current_point(cr, &x, &y);
    cairo_text_extents(cr, t, &extents);

    cairo_move_to(cr, x - extents.x_advance, y);
    cairo_show_text(cr, t);
}

// Centre text
void cairo_centre_text(cairo_t *cr, char *t)
{
    double x, y;
    cairo_text_extents_t extents;

    cairo_get_current_point(cr, &x, &y);
    cairo_text_extents(cr, t, &extents);

    cairo_move_to(cr, x - extents.width / 2, y);
    cairo_show_text(cr, t);
}

// Scope draw callback
gboolean scope_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer)
{
    cairo_edge(cr,  gtk_widget_get_allocated_width(widget),
	       gtk_widget_get_allocated_height(widget));

    int width = gtk_widget_get_allocated_width(widget) - 4;
    int height = gtk_widget_get_allocated_height(widget) - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_translate(cr, 0, height / 2);
    cairo_set_source_rgb(cr, 0, 0.5, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_set_line_width(cr, 1);

    for (int x = 0; x < width; x += 5)
    {
	cairo_move_to(cr, x, -height / 2);
	cairo_line_to(cr, x, height / 2);
    }

    for (int y = 0; y < height / 2; y += 5)
    {
	cairo_move_to(cr, 0, y);
	cairo_line_to(cr, width, y);
	cairo_move_to(cr, 0, -y);
	cairo_line_to(cr, width, -y);
    }

    cairo_stroke(cr);

    // Don't attempt the trace until there's a buffer
    if (scope.data == NULL)
	return true;

    // Initialise sync
    int maxdx = 0;
    int dx = 0;
    int n = 0;

    for (int i = 1; i < width; i++)
    {
	dx = scope.data[i] - scope.data[i - 1];
	if (maxdx < dx)
	{
	    maxdx = dx;
	    n = i;
	}

	if (maxdx > 0 && dx < 0)
	    break;
    }

    static int max;

    if (max < 4096)
	max = 4096;

    double yscale = (double) max / (height / 2);

    max = 0;

    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);

    cairo_move_to(cr, 0, 0);
    for (int i = 0; i < width; i++)
    {
	if (max < abs(scope.data[n + i]))
	    max = abs(scope.data[n + i]);

	double y = -scope.data[n + i] / yscale;
	cairo_line_to(cr, i, y);
    }

    cairo_stroke(cr);

    // Show F for filter
    if (audio.filter)
    {
	cairo_set_source_rgb(cr, 1, 1, 0);
	cairo_move_to(cr, 2, -(height / 2) + 10);
	cairo_show_text(cr, "F");
    }

    return true;
}

// Spectrum draw callback
gboolean spectrum_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer)
{
    static char s[16];

    enum
    {FONT_HEIGHT   = 10};

    cairo_edge(cr,  gtk_widget_get_allocated_width(widget),
	       gtk_widget_get_allocated_height(widget));

    int width = gtk_widget_get_allocated_width(widget) - 4;
    int height = gtk_widget_get_allocated_height(widget) - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_translate(cr, 0, height - 1);
    cairo_set_source_rgb(cr, 0, 0.5, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_set_line_width(cr, 1);

    for (int x = 0; x < width; x += 5)
    {
	cairo_move_to(cr, x, -height);
	cairo_line_to(cr, x, 0);
    }

    for (int y = 0; y < height; y += 5)
    {
	cairo_move_to(cr, 0, -y);
	cairo_line_to(cr, width, -y);
    }

    cairo_stroke(cr);

    // Don't attempt the trace until there's a buffer
    if (spectrum.data == NULL)
	return true;

    // Green pen for spectrum trace
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);

    static double max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling
    double yscale = height / max;

    max = 0.0;

    // Draw the spectrum
    cairo_move_to(cr, 0, 0);

    if (spectrum.zoom)
    {
	// Calculate scale
	double xscale = ((double) width / (spectrum.r - spectrum.l)) / 2.0;

	for (int i = floor(spectrum.l); i <= ceil(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		double value = spectrum.data[i];

		if (max < value)
		    max = value;

		double y = -value * yscale;
		double x = ((double) i - spectrum.l) * xscale; 

		cairo_line_to(cr, x, y);
	    }
	}

        cairo_stroke_preserve(cr);

        cairo_line_to(cr, width, 0);
        cairo_close_path(cr);
        cairo_set_source_rgba(cr, 0, 1, 0, 0.25);
        cairo_fill(cr);

	cairo_move_to(cr, width / 2, 0);
	cairo_line_to(cr, width / 2, -height);

        cairo_set_source_rgb(cr, 0, 1, 0);
	cairo_stroke(cr);

	// Yellow pen for frequency trace
	cairo_set_source_rgb(cr, 1, 1, 0);

	// Draw lines for each frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each that are in range
	    if (spectrum.values[i].f > spectrum.l &&
		spectrum.values[i].f < spectrum.h)
	    {
		double x = (spectrum.values[i].f - spectrum.l) * xscale;
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, -height);

		double f = display.maxima[i].f;

		// Reference freq
		double fr = display.maxima[i].fr;
		double c = -12.0 * log2(fr / f);

		// Ignore silly values
		if (!isfinite(c))
		    continue;

		sprintf(s, "%+0.0f", c * 100.0);
		cairo_move_to(cr, x, -2);
		cairo_centre_text(cr, s);
	    }
	}
    }

    else
    {
	double xscale = ((double) spectrum.length /
                         (double) spectrum.expand) / (double) width;

	for (int x = 0; x < width; x++)
	{
	    double value = 0.0;

	    // Don't show DC component
	    if (x > 0)
	    {
		for (int j = 0; j < xscale; j++)
		{
		    int n = x * xscale + j;

		    if (value < spectrum.data[n])
			value = spectrum.data[n];
		}
	    }

	    if (max < value)
		max = value;

	    double y = -value * yscale;

	    cairo_line_to(cr, x, y);
	}

	cairo_stroke_preserve(cr);

        cairo_line_to(cr, width, 0);
        cairo_close_path(cr);
        cairo_set_source_rgba(cr, 0, 1, 0, 0.25);
        cairo_fill(cr);

	// Yellow pen for frequency trace
	cairo_set_source_rgb(cr, 1, 1, 0);

	// Draw lines for each frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each
	    double x = spectrum.values[i].f / xscale;
	    cairo_move_to(cr, x, 0);
	    cairo_line_to(cr, x, -height);

	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;
	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    sprintf(s, "%+0.0f", c * 100.0);
	    cairo_move_to(cr, x, -2);
	    cairo_centre_text(cr, s);
	}

	if (spectrum.expand > 1)
	{
	    sprintf(s, "x%d", spectrum.expand);
	    cairo_move_to(cr, 0, -2);
	    cairo_show_text(cr, s);
	}
    }

    cairo_stroke(cr);

    // Show D for downsample
    if (audio.downsample)
    {
	cairo_move_to(cr, 0, 10 - height);
	cairo_show_text(cr, "D");
    }

    // cairo_destroy(cr);

    return true;
}

// Display draw callback
gboolean display_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer)
{
    static char s[16];

    static const char *notes[] =
	{"C", "C", "D", "E", "E", "F",
	 "F", "G", "A", "A", "B", "B"};

    static const char *sharps[] =
	{"", "\u266F", "", "\u266D", "", "",
	 "\u266F", "", "\u266D", "", "\u266D", ""};

    cairo_edge(cr,  gtk_widget_get_allocated_width(widget),
	       gtk_widget_get_allocated_height(widget));

    const int width = gtk_widget_get_allocated_width(widget) - 4;
    const int height = gtk_widget_get_allocated_height(widget) - 4;

    const int large = height / 3;
    const int xlarge = height / 2;
    const int medium = height / 5;
    const int small = height / 8;
    const int half = height / 4;

    cairo_set_source_rgb(cr, 0, 0, 0);

    if (display.multiple)
    {
	// Select font
	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, small);

	// Set text align
	if (display.count == 0)
	{
	    // Display note
	    sprintf(s, "%s%s%d", notes[display.n % Length(notes)],
		    sharps[display.n % Length(notes)], display.n / 12);
	    cairo_move_to(cr, 8, small);
	    cairo_show_text(cr, s);

	    // Display cents
	    sprintf(s, " %+4.2lf\u00A2", display.c * 100.0);
	    cairo_show_text(cr, s);

	    // Display reference
	    sprintf(s, " %4.2lfHz", display.fr);
	    cairo_show_text(cr, s);

	    // Display frequency
	    sprintf(s, " %4.2lfHz", display.f);
	    cairo_show_text(cr, s);

	    // Display difference
	    sprintf(s, " %+4.2lfHz", display.f - display.fr);
	    cairo_show_text(cr, s);
	}

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    // Display note
	    sprintf(s, "%s%s%d", notes[n % Length(notes)],
		    sharps[n % Length(notes)], n / 12);
	    cairo_move_to(cr, 8, small + (small * i));
	    cairo_show_text(cr, s);

	    // Display cents
	    sprintf(s, " %+4.2lf\u00A2", c * 100.0);
	    cairo_show_text(cr, s);

	    // Display reference
	    sprintf(s, " %4.2lfHz", fr);
	    cairo_show_text(cr, s);

	    // Display frequency
	    sprintf(s, " %4.2lfHz", f);
	    cairo_show_text(cr, s);

	    // Display difference
	    sprintf(s, " %+4.2lfHz", f - fr);
	    cairo_show_text(cr, s);

	    if (i == 5)
		break;
	}
    }

    else
    {
	double x, y;
	cairo_matrix_t matrix;

	// Select xlarge font
	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);
	cairo_matrix_init_scale(&matrix, 0.8, 1.0);
	cairo_set_font_matrix(cr, &matrix);
	cairo_set_font_size(cr, xlarge);

	// Display note
	sprintf(s, "%s", notes[display.n % Length(notes)]);
	cairo_move_to(cr, 8, xlarge);
	cairo_show_text(cr, s);

	cairo_get_current_point(cr, &x, &y);

	// Select medium font
	cairo_set_font_size(cr, half);

	sprintf(s, "%d", display.n / 12);
	cairo_show_text(cr, s);

        // Save context
	cairo_save(cr);

	sprintf(s, "%s", sharps[display.n % Length(sharps)]);
	cairo_move_to(cr, x, half);
	cairo_show_text(cr, s);

	// Select large font
	cairo_restore(cr);
	cairo_set_font_size(cr, large);

	// Display cents
	sprintf(s, "%+4.2f\u00A2", display.c * 100.0);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);

	y += medium;

	// Select medium font
	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, medium);

	// Display reference frequency
	sprintf(s, "%4.2fHz", display.fr);
	cairo_move_to(cr, 8, y);
	cairo_show_text(cr, s);

	// Display actual frequency
	sprintf(s, "%4.2fHz", display.f);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);

	y += medium;

	// Display reference
	sprintf(s, "%4.2fHz", audio.reference);
	cairo_move_to(cr, 8, y);
	cairo_show_text(cr, s);

	// Display frequency difference
	sprintf(s, "%+4.2lfHz", display.f - display.fr);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);
    }

    // Show lock
    if (display.lock)
	// DrawLock(hbdc, -1, height + 1);
	;

    // cairo_destroy(cr);

    return true;
}

// Strobe draw callback
gboolean strobe_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer)
{
    static cairo_pattern_t *cp;

    // Colours
    static double colours[][2][3] =
	{{{0.25, 0.25, 1.0}, {0.25, 1.0, 1.0}},
	 {{0.5, 0.5, 0}, {0.5, 1.0, 0.828}},
	 {{1.0, 0.25, 1.0}, {1.0, 1.0, 0.25}}};

    cairo_edge(cr,  gtk_widget_get_allocated_width(widget),
	       gtk_widget_get_allocated_height(widget));

    if (!strobe.enable)
	return true;

    int height = gtk_widget_get_allocated_height(widget) - 4;

    if (cp == NULL || strobe.changed)
    {
	if (cp != NULL)
	    cairo_pattern_destroy(cp);

	cairo_surface_t *cs =
	    cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				       height / 4 * 16, height);

	cairo_t *cr = cairo_create(cs);

	cairo_set_source_rgb(cr, colours[strobe.colours][1][0],
			     colours[strobe.colours][1][1],
			     colours[strobe.colours][1][2]);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, colours[strobe.colours][0][0],
			     colours[strobe.colours][0][1],
			     colours[strobe.colours][0][2]);

	for (int i = 0; i < 8; i++)
	{
	    cairo_rectangle(cr,  i * height / 2, 0, height / 4, height / 4);
	    cairo_fill(cr);
	}

	for (int i = 0; i < 4; i++)
	{
	    cairo_rectangle(cr, i * height, height / 4, height / 2, height / 4);
	    cairo_fill(cr);
	}

	for (int i = 0; i < 2; i++)
	{
	    cairo_rectangle(cr, i * height * 2, height / 2, height, height / 4);
	    cairo_fill(cr);
	}

	cairo_rectangle(cr, 0, height * 3 / 4, height * 2, height / 4);
	cairo_fill(cr);

	cairo_destroy(cr);

	cp = cairo_pattern_create_for_surface(cs);
	cairo_pattern_set_extend(cp, CAIRO_EXTEND_REPEAT);
	cairo_surface_destroy(cs);

	strobe.changed = false;
    }

    static float mc = 0.0;
    static float mx = 0.0;

    mc = ((7.0 * mc) + strobe.c) / 8.0;
    mx += mc * 50.0;

    if (mx > height * 4)
	mx = 0.0;

    if (mx < 0.0)
	mx = height * 4;

    cairo_matrix_t m;

    cairo_matrix_init_translate(&m, mx - height * 4, 0);
    cairo_pattern_set_matrix(cp, &m);

    cairo_set_source(cr, cp);
    cairo_paint(cr);
    // cairo_destroy(cr);

    return true;
}

// Meter draw callback
gboolean meter_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer)
{
    cairo_edge(cr,  gtk_widget_get_allocated_width(widget),
               gtk_widget_get_allocated_height(widget));

    int width = gtk_widget_get_allocated_width(widget) - 4;
    int height = gtk_widget_get_allocated_height(widget) - 4;

    // Select font
    cairo_select_font_face(cr, "sans-serif",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, height / 3);

    // Move origin
    cairo_translate(cr, width / 2, 0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);

    // Draw the meter scale
    for (int i = 0; i < 6; i++)
    {
	static char s[16];
	int x = width / 11 * i;

        cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	sprintf(s, "%d", i * 10);
	cairo_move_to(cr, x, height / 3);
	cairo_centre_text(cr, s);
	cairo_move_to(cr, -x, height / 3);
	cairo_centre_text(cr, s);

	cairo_move_to(cr, x, height / 3);
	cairo_line_to(cr, x, height / 2);
	cairo_move_to(cr, -x, height / 3);
	cairo_line_to(cr, -x, height / 2);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		cairo_move_to(cr, x + j * width / 55, height * 3 / 8);
		cairo_line_to(cr, x + j * width / 55, height / 2);
	    }

	    cairo_move_to(cr, -x + j * width / 55, height * 3 / 8);
	    cairo_line_to(cr, -x + j * width / 55, height / 2);
	}
    }

    cairo_stroke(cr);

    cairo_pattern_t *linear =
        cairo_pattern_create_linear(0, height * 3 / 4, 0, height * 3 / 4 + 4);
    cairo_pattern_add_color_stop_rgb(linear, 0, 0.5, 0.5, 0.5);
    cairo_pattern_add_color_stop_rgb(linear, 4, 1, 1, 1);

    cairo_set_source(cr, linear);
    cairo_rectangle(cr, -(width - 20) / 2, height * 3 / 4, width - 20, 2);

    cairo_stroke(cr);

    static float mc;

    // Do calculation
    mc = ((mc * 3.0) + meter.c) / 4.0;

    // Calculate x
    double x = width / 11 * mc * 10;
    cairo_translate(cr, x, height * 3 / 4);

    linear = cairo_pattern_create_linear(0, 0, 0, 12);
    cairo_pattern_add_color_stop_rgb(linear, 0, 1, 1, 1);
    cairo_pattern_add_color_stop_rgb(linear, 12, 0, 0, 0);
    cairo_pattern_set_extend(linear, CAIRO_EXTEND_REFLECT);

    cairo_set_source(cr, linear);

    cairo_move_to(cr, 0, -10);
    cairo_rel_line_to(cr, -5, 5);
    cairo_rel_line_to(cr, 0, 13);
    cairo_rel_line_to(cr, 10, 0);
    cairo_rel_line_to(cr, 0, -13);
    cairo_rel_line_to(cr, -5, -5);

    // cairo_scale(cr, height / 24, height / 24);
    cairo_fill_preserve(cr);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1);

    cairo_stroke(cr);

    return true;
}

// Update options
void update_options()
{
    if (options.widget == NULL)
	return;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.filter),
				 audio.filter);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.downsample),
				 audio.downsample);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.lock),
				 display.lock);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.multiple),
				 display.multiple);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.zoom),
				 spectrum.zoom);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.strobe),
				 strobe.enable);
}

// Key press

gboolean key_press(GtkWidget *window, GdkEventKey *event, gpointer data)
{
    switch (event->keyval)
    {
    case GDK_KEY_c:
    case GDK_KEY_C:
	break;

    case GDK_KEY_d:
    case GDK_KEY_D:
	audio.downsample = !audio.downsample;
	break;

    case GDK_KEY_f:
    case GDK_KEY_F:
	audio.filter = !audio.filter;
	break;

    case GDK_KEY_k:
    case GDK_KEY_K:
	strobe.colours++;
	if (strobe.colours > MAGENTA)
	    strobe.colours = BLUE;

	strobe.changed = true;
	break;

    case GDK_KEY_l:
    case GDK_KEY_L:
	display.lock = !display.lock;
	break;

    case GDK_KEY_m:
    case GDK_KEY_M:
	display.multiple = !display.multiple;
	break;

    case GDK_KEY_s:
    case GDK_KEY_S:
	strobe.enable = !strobe.enable;
	break;

    case GDK_KEY_z:
    case GDK_KEY_Z:
	spectrum.zoom = !spectrum.zoom;
	break;

    case GDK_KEY_plus:
    case GDK_KEY_equal:
    case GDK_KEY_KP_Add:
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;
	break;

    case GDK_KEY_minus:
    case GDK_KEY_underscore:
    case GDK_KEY_KP_Subtract:
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;
	break;

    default:
	printf("Key code = %x\n", event->keyval);
	return true;
    }

    // Update options
    update_options();

    return true;
}

// Reference changed
void reference_changed(GtkWidget widget, gpointer data)
{
    audio.reference =
        gtk_spin_button_get_value(GTK_SPIN_BUTTON(options.reference));
}

// Filter clicked
void filter_clicked(GtkWidget widget, gpointer data)
{
    audio.filter =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.filter));
}

// Lock clicked
void lock_clicked(GtkWidget widget, gpointer data)
{
    display.lock =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.lock));
}

// Strobe clicked
void strobe_clicked(GtkWidget widget, gpointer data)
{
    strobe.enable =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.strobe));
}

// Downsample clicked
void downsample_clicked(GtkWidget widget, gpointer data)
{
    audio.downsample =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.downsample));
}

// Multiple clicked
void multiple_clicked(GtkWidget widget, gpointer data)
{
    display.multiple =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.multiple));;
}

// Zoom clicked
void zoom_clicked(GtkWidget widget, gpointer data)
{
    spectrum.zoom =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options.zoom));
}

// Correction changed
void correction_changed(GtkWidget widget, gpointer data)
{
    audio.correction =
        gtk_spin_button_get_value(GTK_SPIN_BUTTON(options.correction));
}

// Widget clicked
void widget_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    const char *name;

    name = gtk_widget_get_name(widget);

    if (strcmp(name, "scope") == 0)
	audio.filter = !audio.filter;

    if (strcmp(name, "spectrum") == 0)
	spectrum.zoom = !spectrum.zoom;

    if (strcmp(name, "display") == 0)
	display.lock = !display.lock;

    if (strcmp(name, "strobe") == 0)
	strobe.enable = !strobe.enable;

    if (strcmp(name, "meter") == 0)
	display.lock = !display.lock;

    update_options();
}

// Options menu
void options_menu(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    GtkWidget *menu;
    GtkWidget *item;

    // Menu
    menu = gtk_menu_new();

    // Downsample
    item = gtk_check_menu_item_new_with_label("Downsample audio");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), audio.downsample);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Filter
    item = gtk_check_menu_item_new_with_label("Filter audio");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), audio.filter);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Lock
    item = gtk_check_menu_item_new_with_label("Display lock");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), display.lock);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Multiple
    item = gtk_check_menu_item_new_with_label("Multiple notes");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), display.multiple);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Strobe
    item = gtk_check_menu_item_new_with_label("Show strobe");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), strobe.enable);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Zoom
    item = gtk_check_menu_item_new_with_label("Zoom spectrum");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), spectrum.zoom);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Separator
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Options
    item = gtk_menu_item_new_with_label("Options\u2026");
    g_signal_connect(G_OBJECT(item), "activate",
		     G_CALLBACK(options_clicked), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Separator
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Quit
    item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(G_OBJECT(item), "activate",
		     G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

// Button press
gboolean button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    // Check button
    switch (event->button)
    {
    case 1:
	widget_clicked(widget, event, data);
	break;

    case 2:
	return false;
	break;

    case 3:
	options_menu(widget, event, data);
	break;

    default:
	return false;
	break;
    }

    return true;
}

// Fundamental clicked
void fund_toggled(GtkWidget *widget, gpointer data)
{
    filters.fund =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

// Note clicked
void note_toggled(GtkWidget *widget, gpointer data)
{
    filters.note =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

// Expand changed
void expand_changed(GtkWidget *widget, gpointer data)
{
}

// Note filter callback
void note_clicked(GtkWidget *widget, GtkWindow *window)
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *fund;
    GtkWidget *note;
    GtkWidget *separator;

    if (options.note != NULL)
    {
	gtk_widget_show_all(options.note);
	return;
    }

    // Create note filter dialog
    options.note = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(options.note), "Tuner note filter");
    gtk_window_set_resizable(GTK_WINDOW(options.note), false);
    gtk_window_set_transient_for(GTK_WINDOW(options.note), window);

    // V box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(options.note), vbox);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, true, true, MARGIN);

    // Fundamental
    fund = gtk_check_button_new_with_label("Fundamental filter");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fund),
				 filters.fund);
    gtk_box_pack_start(GTK_BOX(hbox), fund, true, true, MARGIN);

    // Fundamental clicked
    g_signal_connect(G_OBJECT(fund), "toggled",
		     G_CALLBACK(fund_toggled), window);

    // Note
    note = gtk_check_button_new_with_label("Note filter");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(note),
				 filters.note);
    gtk_box_pack_start(GTK_BOX(hbox), note, true, true, MARGIN);

    // Note clicked
    g_signal_connect(G_OBJECT(note), "toggled",
		     G_CALLBACK(note_toggled), window);

    // Separator
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator, false, false, 0);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, MARGIN);

    // Destroy dialog callback
    g_signal_connect(G_OBJECT(options.note), "destroy",
		     G_CALLBACK(gtk_widget_destroyed), &options.note);

    gtk_widget_show_all(options.note);
}

// Options callback
void options_clicked(GtkWidget *widget, GtkWindow *window)
{
    GtkWidget *hbox;
    GtkWidget *ibox;
    GtkWidget *vbox;
    GtkWidget *note;
    GtkWidget *close;
    GtkWidget *label;
    GtkWidget *save;
    GtkWidget *separator;

    if (options.widget != NULL)
    {
	gtk_widget_show_all(options.widget);
	return;
    }

    // Create options dialog
    options.widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(options.widget), "Tuner Options");
    gtk_window_set_resizable(GTK_WINDOW(options.widget), false);
    gtk_window_set_transient_for(GTK_WINDOW(options.widget), window);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_container_add(GTK_CONTAINER(options.widget), hbox);

    // V box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, MARGIN);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, MARGIN);

    // I box
    ibox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ibox, false, false, 0);

    // Zoom
    options.zoom = gtk_check_button_new_with_label("Zoom spectrum");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.zoom),
				 spectrum.zoom);
    gtk_box_pack_start(GTK_BOX(ibox), options.zoom, false, false, 0);

    // Zoom clicked
    g_signal_connect(G_OBJECT(options.zoom), "toggled",
		     G_CALLBACK(zoom_clicked), window);
    // Filter
    options.filter = gtk_check_button_new_with_label("Filter audio");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.filter),
				 audio.filter);
    gtk_box_pack_start(GTK_BOX(ibox), options.filter, false, false, 0);

    // Filter clicked
    g_signal_connect(G_OBJECT(options.filter), "toggled",
		     G_CALLBACK(filter_clicked), window);
    // Multiple
    options.multiple = gtk_check_button_new_with_label("Multiple notes");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.multiple),
				 display.multiple);
    gtk_box_pack_start(GTK_BOX(ibox), options.multiple, false, false, 0);

    // Multiple clicked
    g_signal_connect(G_OBJECT(options.multiple), "toggled",
		     G_CALLBACK(multiple_clicked), window);
    // Fundamental
    options.fundamental = gtk_check_button_new_with_label("Fundamental");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.fundamental),
				 audio.fundamental);
    gtk_box_pack_start(GTK_BOX(ibox), options.fundamental, false, false, 0);

    // Fundamental clicked
    g_signal_connect(G_OBJECT(options.fundamental), "toggled",
        	     G_CALLBACK(fund_toggled), window);

    gtk_box_pack_start(GTK_BOX(hbox), ibox, false, false, MARGIN);

    // V box
    ibox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ibox, true, true, MARGIN);

    // Strobe
    options.strobe = gtk_check_button_new_with_label("Show strobe");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.strobe),
				 strobe.enable);
    gtk_box_pack_start(GTK_BOX(ibox), options.strobe, false, false, 0);

    // Strobe clicked
    g_signal_connect(G_OBJECT(options.strobe), "toggled",
		     G_CALLBACK(strobe_clicked), window);

    // Downsample
    options.downsample = gtk_check_button_new_with_label("Downsample audio");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.downsample),
				 audio.downsample);
    gtk_box_pack_start(GTK_BOX(ibox), options.downsample, false, false, 0);

    // Downsample clicked
    g_signal_connect(G_OBJECT(options.downsample), "toggled",
		     G_CALLBACK(downsample_clicked), window);
    // Lock
    options.lock = gtk_check_button_new_with_label("Lock display");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.lock),
				 display.lock);
    gtk_box_pack_start(GTK_BOX(ibox), options.lock, false, false, 0);

    // Lock clicked
    g_signal_connect(G_OBJECT(options.lock), "toggled",
		     G_CALLBACK(lock_clicked), window);
    // Note
    options.note = gtk_check_button_new_with_label("Note filter");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.note),
				 audio.note);
    gtk_box_pack_start(GTK_BOX(ibox), options.note, false, false, 0);

    // Note clicked
    g_signal_connect(G_OBJECT(options.note), "toggled",
		     G_CALLBACK(note_toggled), window);

    gtk_box_pack_start(GTK_BOX(hbox), ibox, false, false, 0);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, false, false, 0);

    // Label
    label = gtk_label_new("Expand");
    gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);

    options.expand = gtk_combo_box_text_new();
    const char *expansions[] =
        {"x 1", "x 2", "x 4", "x 8", "x 16"};
    for (unsigned int i = 0; i < Length(expansions); i++)
    {
        char s[16];
        sprintf(s, "%d", i);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(options.expand),
                                  s, expansions[i]);
    }
    gtk_box_pack_end(GTK_BOX(hbox), options.expand, false, false, 0);

    // Expand changed
    g_signal_connect(G_OBJECT(options.note), "changed",
		     G_CALLBACK(expand_changed), window);

    gtk_box_pack_start(GTK_BOX(vbox), vbox, false, false, 0);

    /*
    // Close button
    close = gtk_button_new_with_label("  Close  ");
    gtk_box_pack_end(GTK_BOX(hbox), close, false, false, MARGIN);

    // Close clicked
    g_signal_connect_swapped(G_OBJECT(close), "clicked",
			     G_CALLBACK(gtk_widget_hide), options.widget);
    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, false, false, 0);

    // Label
    label = gtk_label_new("Correction:");
    gtk_box_pack_start(GTK_BOX(hbox), label, false, false, MARGIN);

    // Correction
    options.correction = gtk_spin_button_new_with_range(0.9999, 1.0001,
							0.000001);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(options.correction),
			      audio.correction);
    gtk_box_pack_start(GTK_BOX(hbox), options.correction, false, false, 0);

    // Correction changed
    g_signal_connect(G_OBJECT(options.correction), "value-changed",
		     G_CALLBACK(correction_changed), window);

    // Save button
    save = gtk_button_new_with_label("   Save   ");
    gtk_box_pack_end(GTK_BOX(hbox), save, false, false, MARGIN);

    // Save clicked
    g_signal_connect(G_OBJECT(save), "clicked",
		     G_CALLBACK(save_clicked), window);
    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, false, false, MARGIN);

    // Label
    label = gtk_label_new("Use correction if your sound card clock is\n"
			  "significantly inaccurate");
    gtk_box_pack_start(GTK_BOX(hbox), label, false, false, MARGIN);

    // Separator
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_end(GTK_BOX(vbox), separator, false, false, 0);

    // H box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
    // I box
    ibox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), ibox, false, false, 0);

    // Label
    label = gtk_label_new("Reference:");
    gtk_box_pack_start(GTK_BOX(ibox), label, false, false, 0);

    // Reference
    options.reference = gtk_spin_button_new_with_range(430.0, 450.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(options.reference),
			      audio.reference);
    gtk_box_pack_start(GTK_BOX(ibox), options.reference, false, false, 0);

    // Reference changed
    g_signal_connect(G_OBJECT(options.reference), "value-changed",
		     G_CALLBACK(reference_changed), window);
    // I box
    ibox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), ibox, false, false, 0);

    // Note filter
    note = gtk_button_new_with_label("  Note\u2026  ");
    gtk_box_pack_end(GTK_BOX(ibox), note, false, false, 0);

    // Note clicked
    g_signal_connect(G_OBJECT(note), "clicked",
		     G_CALLBACK(note_clicked), window);

    // Destroy dialog callback
    g_signal_connect(G_OBJECT(options.widget), "destroy",
		     G_CALLBACK(gtk_widget_destroyed), &options.widget);
    */
    gtk_widget_show_all(options.widget);
}

// Save callback
void save_clicked(GtkWidget *widget, GtkWindow *window)
{
    audio.save = true;
}