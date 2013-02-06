////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - An Android Tuner written in Java.
//
//  Copyright (C) 2013	Bill Farmer
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
//  Bill Farmer	 william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

package org.billthefarmer.tuner;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.os.Bundle;
import android.preference.PreferenceManager;

import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.Toast;

// Main Activity

public class MainActivity extends Activity
{
    Spectrum spectrum;
    Display display;
    Strobe strobe;
    Status status;
    Meter meter;
    Scope scope;

    Audio audio;

    // On Create

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
	super.onCreate(savedInstanceState);
	setContentView(R.layout.activity_main);

	// Find the views, not all may be present

	spectrum = (Spectrum)findViewById(R.id.spectrum);
	display = (Display)findViewById(R.id.display);
	strobe = (Strobe)findViewById(R.id.strobe);
	status = (Status)findViewById(R.id.status);
	meter = (Meter)findViewById(R.id.meter);
	scope = (Scope)findViewById(R.id.scope);

	// Create audio

	audio = new Audio();

	// Connect views to audio

	if (spectrum != null)
	    spectrum.audio = audio;

	if (display != null)
	    display.audio = audio;

	if (strobe != null)
	    strobe.audio = audio;

	if (status != null)
	    status.audio = audio;

	if (meter != null)
	    meter.audio = audio;

	if (scope != null)
	    scope.audio = audio;

	// Set up the click listeners

	setClickListeners();
    }

    // No menu yet

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
	// Inflate the menu; this adds items to the action bar if it is present.

	MenuInflater inflater = getMenuInflater();
	inflater.inflate(R.menu.activity_main, menu);
	return true;
    }

    // Set click listeners

    void setClickListeners()
    {
	// Scope

	if (scope != null)
	    scope.setOnClickListener(new OnClickListener()
		{
		    @Override
		    public void onClick(View v)
		    {
			audio.filter = !audio.filter;

			if (audio.filter)
			    showToast(R.string.filter_on);
			else
			    showToast(R.string.filter_off);
		    }
		});

	// Spectrum

	if (spectrum != null)
	{
	    spectrum.setOnClickListener(new OnClickListener()
		{
		    @Override
		    public void onClick(View v)
		    {
			audio.zoom = !audio.zoom;

			if (audio.zoom)
			    showToast(R.string.zoom_on);
			else
			    showToast(R.string.zoom_off);
		    }
		});

	    spectrum.setOnLongClickListener(new OnLongClickListener()
		{
		    @Override
		    public boolean onLongClick(View v)
		    {
			audio.downsample = !audio.downsample;

			if (audio.downsample)
			    showToast(R.string.down_on);
			else
			    showToast(R.string.down_off);

		    return true;
		    }
		});
	}

	// Display

	if (display != null)
	{
		display.setOnClickListener(new OnClickListener()
		{
		    @Override
		    public void onClick(View v)
		    {
			audio.lock = !audio.lock;

			if (audio.lock)
			    showToast(R.string.lock_on);
			else
			    showToast(R.string.lock_off);
		    }
		});

		display.setOnLongClickListener(new OnLongClickListener()
		{
			@Override
			public boolean onLongClick(View v)
			{
				audio.multiple = !audio.multiple;

				if (audio.multiple)
					showToast(R.string.multiple_on);

				else
					showToast(R.string.multiple_off);

				return true;
			}
		});
	}

	// Strobe

	if (strobe != null)
	    strobe.setOnClickListener(new OnClickListener()
		{
		    @Override
		    public void onClick(View v)
		    {
			audio.strobe = !audio.strobe;

			if (audio.strobe)
			    showToast(R.string.strobe_on);
			else
			    showToast(R.string.strobe_off);
		    }
		});
    }
    
    // Show toast

    void showToast(int key)
    {
    	Resources resources = getResources();
    	String text = resources.getString(key);

    	Toast toast = Toast.makeText(this, text, Toast.LENGTH_SHORT);

	toast.setGravity(Gravity.CENTER, 0, 0);
	toast.show();
    }

    // On start

    @Override
    protected void onStart()
    {
	super.onStart();
    }

    // On Resume

    @Override
    protected void onResume()
    {
	super.onResume();

	// Get preferences

	getPreferences();

	// Update status

	if (status != null)
	    status.invalidate();

	// Start the audio thread

	audio.start();
    }

    @Override
    protected void onPause()
    {
	super.onPause();

	// Save resources

	savePreferences();

	// Stop audio thread

	audio.stop();
    }

    // On stop

    @Override
    protected void onStop()
    {
	super.onStop();
    }
    // On settings click

    public void onSettingsClick(MenuItem item)
    {
	Intent intent = new Intent(this, SettingsActivity.class);
	startActivity(intent);
    }

    // Save preferences

    void savePreferences()
    {
    	SharedPreferences preferences =
    		    PreferenceManager.getDefaultSharedPreferences(this);

    	Editor editor = preferences.edit();

    	editor.putBoolean("pref_filter", audio.filter);
    	editor.putBoolean("pref_down", audio.downsample);
    	editor.putBoolean("pref_multiple", audio.multiple);
    	editor.putBoolean("pref_strobe", audio.strobe);
    	editor.putBoolean("pref_zoom", audio.zoom);

    	editor.commit();
    }

    // Get preferences

    void getPreferences()
    {
	// Load preferences

	PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

	SharedPreferences preferences =
	    PreferenceManager.getDefaultSharedPreferences(this);

	// Set preferences

	if (audio != null)
	{
		audio.sample = preferences.getInt("pref_source", 0);
	    audio.reference = preferences.getInt("pref_reference", 440);

	    audio.sample =
		Double.valueOf(preferences.getString("pref_sample", "11025"));

	    audio.filter = preferences.getBoolean("pref_filter", true);
	    audio.downsample = preferences.getBoolean("pref_down", false);
	    audio.multiple = preferences.getBoolean("pref_multiple", false);
	    audio.strobe = preferences.getBoolean("pref_strobe", true);
	    audio.zoom = preferences.getBoolean("pref_zoom", true);
	}
    }

    // Show alert

    void showAlert(String title, String message)
    {
	// Create an alert dialog builder

	AlertDialog.Builder builder =
	    new AlertDialog.Builder(this);

	// Set the title, message and button

	builder.setTitle(title);
	builder.setMessage(message);
	builder.setNeutralButton(android.R.string.ok,
				 new DialogInterface.OnClickListener()
				 {				
				     @Override
				     public void onClick(DialogInterface dialog,
							 int which)
				     {
					 // Dismiss dialog

					 dialog.dismiss();	
				     }
				 });
	// Create the dialog

	AlertDialog dialog = builder.create();

	// Show it

	dialog.show();
    }

    // Audio

    protected class Audio
    {
	// Preferences

	protected int source;

	protected boolean lock;
	protected boolean zoom;
	protected boolean filter;
	protected boolean strobe;
	protected boolean multiple;
	protected boolean downsample;

	protected double reference;
	protected double sample;

	// Data

	protected Thread thread;
	protected double buffer[];
	protected short data[];

	// Output data

	protected double lower;
	protected double higher;
	protected double nearest;
	protected double frequency;
	protected double difference;
	protected double cents;
	protected double fps;

	protected int count;
	protected int n;

	// Private data

	private long timer;

	private AudioRecord audioRecord;

	private static final int MAXIMA = 8;
	private static final int OVERSAMPLE = 16;
	private static final int SAMPLES = 16384;
	private static final int RANGE = SAMPLES * 3 / 8;
	private static final int STEP = SAMPLES / OVERSAMPLE;

	private static final int C5_OFFSET = 57;
	private static final long TIMER_COUNT = 16; 
	private static final double MIN = 0.5;

	private static final double G = 3.023332184e+01;
	private static final double K = 0.9338478249;

	private double xv[];
	private double yv[];

	private double dmax;

	private Complex x;

	protected double xa[];

	private double xp[];
	private double xf[];
	private double dx[];

	private double x2[];
	private double x3[];
	private double x4[];
	private double x5[];

	protected Maxima maxima;

	// Constructor

	Audio()
	{
	    buffer = new double[SAMPLES];
	    data = new short[STEP];
	    
	    xv = new double[2];
	    yv = new double[2];

	    x = new Complex(SAMPLES);

	    xa = new double[RANGE];
	    xp = new double[RANGE];
	    xf = new double[RANGE];
	    dx = new double[RANGE];

	    x2 = new double[RANGE / 2];
	    x3 = new double[RANGE / 3];
	    x4 = new double[RANGE / 4];
	    x5 = new double[RANGE / 5];

	    maxima = new Maxima(MAXIMA);
	}

	// Start audio

	void start()
	{
	    // Start the thread

	    new Thread(new Runnable()
		{
		    public void run()
		    {
			processAudio();
		    }
		}, "Audio").start();
	}

	// Stop

	void stop()
	{
	    thread = null;
	}

	// Process Audio

	void processAudio()
	{
	    // Save the thread

	    thread = Thread.currentThread();

	    fps = (double)sample / (double)SAMPLES;
	    final double expect = 2.0 * Math.PI *
		(double)STEP / (double)SAMPLES;

	    int size =
		AudioRecord.getMinBufferSize((int)sample,
					     AudioFormat.CHANNEL_IN_MONO,
					     AudioFormat.ENCODING_PCM_16BIT);
	    if (size <= 0)
	    {
		runOnUiThread(new Runnable()
		    {
			public void run()
			{
			    showAlert("Tuner",
				      "Error from AudioRecord.getMinBufferSize()");
			}
		    });

		thread = null;
		return;
	    }

	    // Create the AudioRecord object

	    audioRecord =
		new AudioRecord(source, (int)sample,
				AudioFormat.CHANNEL_IN_MONO,
				AudioFormat.ENCODING_PCM_16BIT, 4096);
	    // Check state

	    int state = audioRecord.getState(); 

	    if (state != AudioRecord.STATE_INITIALIZED)
	    {
		runOnUiThread(new Runnable()
		    {
			public void run()
			{
			    showAlert("Alert!",
				      "Audio record not initialised!");
			}
		    });

		audioRecord.release();
		thread = null;
		return;
	    }

	    // Start recording

	    audioRecord.startRecording();

	    // Continue until the thread is stopped

	    while (thread != null)
	    {
		// Read a buffer of data

		size = audioRecord.read(data, 0, STEP);

		// Stop the thread if no data

		if (size == 0)
		{
		    thread = null;
		    break;
		}

		// Move the main data buffer up

		System.arraycopy(buffer, STEP, buffer, 0, SAMPLES - STEP);

		// Butterworth filter, 3dB/octave

		for (int i = 0; i < STEP; i++)
		{
		    xv[0] = xv[1];
		    xv[1] = (double)data[i] / G;

		    yv[0] = yv[1];
		    yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

		    // Choose filtered/unfiltered data

		    buffer[(SAMPLES - STEP) + i] =
			audio.filter? yv[1]: (double)data[i];
		}

		// Maximum data value

		if (dmax < 4096.0)
		    dmax = 4096.0;

		// Calculate normalising value

		double norm = dmax;
		dmax = 0.0;

		// Copy data to FFT input arrays for tuner

		for (int i = 0; i < SAMPLES; i++)
		{
		    // Find the magnitude

		    if (dmax < Math.abs(buffer[i]))
			dmax = Math.abs(buffer[i]);

		    // Calculate the window

		    double window =
			0.5 - 0.5 * Math.cos(2.0 * Math.PI *
					     i / SAMPLES);

		    // Normalise and window the input data

		    x.r[i] = buffer[i] / norm * window;
		}

		// do FFT for tuner

		fftr(x);

		// Process FFT output for tuner

		for (int i = 1; i < RANGE; i++)
		{
		    double real = x.r[i];
		    double imag = x.i[i];

		    xa[i] = Math.hypot(real, imag);

		    // Do frequency calculation

		    double p = Math.atan2(imag, real);
		    double dp = xp[i] - p;

		    xp[i] = p;

		    // Calculate phase difference

		    dp -= (double)i * expect;

		    int qpd = (int)(dp / Math.PI);

		    if (qpd >= 0)
			qpd += qpd & 1;

		    else
			qpd -= qpd & 1;

		    dp -=  Math.PI * (double)qpd;

		    // Calculate frequency difference

		    double df = OVERSAMPLE * dp / (2.0 * Math.PI);

		    // Calculate actual frequency from slot frequency plus
		    // frequency difference and correction value

		    xf[i] = i * fps + df * fps;

		    // Calculate differences for finding maxima

		    dx[i] = xa[i] - xa[i - 1];
		}

		// Downsample

		if (downsample)
		{
		    // x2 = xa << 2

		    for (int i = 0; i < RANGE / 2; i++)
		    {
			x2[i] = 0.0;

			for (int j = 0; j < 2; j++)
			    x2[i] += xa[(i * 2) + j] / 2.0;
		    }

		    // x3 = xa << 3

		    for (int i = 0; i < RANGE / 3; i++)
		    {
			x3[i] = 0.0;

			for (int j = 0; j < 3; j++)
			    x3[i] += xa[(i * 3) + j] / 3.0;
		    }

		    // x4 = xa << 4

		    for (int i = 0; i < RANGE / 4; i++)
		    {
			x4[i] = 0.0;

			for (int j = 0; j < 4; j++)
			    x2[i] += xa[(i * 4) + j] / 4.0;
		    }

		    // x5 = xa << 5

		    for (int i = 0; i < RANGE / 5; i++)
		    {
			x5[i] = 0.0;

			for (int j = 0; j < 5; j++)
			    x5[i] += xa[(i * 5) + j] / 5.0;
		    }

		    // Add downsamples

		    for (int i = 1; i < RANGE; i++)
		    {
			if (i < RANGE / 2)
			    xa[i] += x2[i];

			if (i < RANGE / 3)
			    xa[i] += x3[i];

			if (i < RANGE / 4)
			    xa[i] += x4[i];

			if (i < RANGE / 5)
			    xa[i] += x5[i];

			// Recalculate differences

			dx[i] = xa[i] - xa[i - 1];
		    }
		}

		// Maximum FFT output

		double max = 0.0;

		count = 0;
		int limit = RANGE - 1;

		// Find maximum value, and list of maxima

		for (int i = 1; i < limit; i++)
		{
		    if (xa[i] > max)
		    {
			max = xa[i];
			frequency = xf[i];
		    }

		    // If display not locked, find maxima and add to list

		    if (!lock && count < MAXIMA &&
			xa[i] > MIN && xa[i] > (max / 4.0) &&
			dx[i] > 0.0 && dx[i + 1] < 0.0)
		    {
			maxima.f[count] = xf[i];

			// Cents relative to reference

			double cf =
			    -12.0 * log2(reference / xf[i]);

			// Reference note

			maxima.r[count] = reference *
			    Math.pow(2.0, Math.round(cf) / 12.0);

			// Note number

			maxima.n[count] = (int)(Math.round(cf) + C5_OFFSET);

			// Set limit to octave above

			if (!downsample && (limit > i * 2))
			    limit = i * 2 - 1;

			count++;
		    }
		}

		// Found flag

		boolean found = false;
		n = 0;

		// Do the note and cents calculations

		if (max > MIN)
		{
		    found = true;

		    // Frequency

		    if (!downsample)
			frequency = maxima.f[0];

		    // Cents relative to reference

		    double cf =
			-12.0 * log2(reference / frequency);

		    // Reference note

		    nearest = audio.reference *
			Math.pow(2.0, Math.round(cf) / 12.0);

		    // Lower and upper freq

		    lower = reference *
			Math.pow(2.0, (Math.round(cf) - 0.55) / 12.0);
		    higher = reference *
			Math.pow(2.0, (Math.round(cf) + 0.55) / 12.0);

		    // Note number

		    n = (int)Math.round(cf) + C5_OFFSET;

		    if (n < 0)
			found = false;

		    // Find nearest maximum to reference note

		    double df = 1000.0;

		    for (int i = 0; i < count; i++)
		    {
			if (Math.abs(maxima.f[i] - nearest) < df)
			{
			    df = Math.abs(maxima.f[i] - nearest);
			    frequency = maxima.f[i];
			}
		    }

		    // Difference

		    difference = frequency - nearest;

		    // Ignore silly values

		    if (Double.isNaN(cf))
			found = false;

		    // Ignore if not within 50 cents of reference note

		    if (Math.abs(cents) > 50.0)
			found = false;
		}

		// If display not locked

		if (!lock)
		{
		    // Update scope window

		    if (scope != null)
			scope.postInvalidate();

		    if (spectrum != null)
			spectrum.postInvalidate();
		}

		// Found

		if (found)
		{
		    // If display not locked

		    if (!lock)
		    {
			// Cents relative to reference note

			cents = -12.0 * log2(nearest / frequency) * 100.0;

			// Update display

			if (display != null)
			    display.postInvalidate();

			if (meter != null)
			    meter.postInvalidate();

			// Update spectrum

			if (spectrum != null)
			    spectrum.postInvalidate();
		    }

		    // Reset count;

		    timer = 0;
		}

		else
		{
		    // If display not locked

		    if (!lock)
		    {
			if (timer > TIMER_COUNT)
			{
			    difference = 0.0;
			    frequency = 0.0;
			    nearest = 0.0;
			    higher = 0.0;
			    lower = 0.0;
			    cents = 0.0;
			    n = 0;

			    // Update spectrum

			    lower = 0.0;
			    higher = 0.0;

			    count = 0;
			}

			// Update spectrum

			if (spectrum != null)
			    spectrum.postInvalidate();

			// Update display

			display.postInvalidate();
			meter.postInvalidate();
		    }
		}

		timer++;
	    }

	    // Stop and release the audio recorder

	    if (audioRecord != null)
	    {
		audioRecord.stop();
		audioRecord.release();
	    }
	}

	// Real to complex FFT, ignores imaginary values in input array

	private void fftr(Complex a)
	{
	    final int n = a.r.length;
	    final double norm = Math.sqrt(1.0 / n);

	    for (int i = 0, j = 0; i < n; i++)
	    {
		if (j >= i)
		{
		    double tr = a.r[j] * norm;

		    a.r[j] = a.r[i] * norm;
		    a.i[j] = 0.0;

		    a.r[i] = tr;
		    a.i[i] = 0.0;
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
		double delta = (Math.PI / mmax);
		for (int m = 0; m < mmax; m++)
		{
		    double w = m * delta;
		    double wr = Math.cos(w);
		    double wi = Math.sin(w);

		    for (int i = m; i < n; i += istep)
		    {
			int j = i + mmax;
			double tr = wr * a.r[j] - wi * a.i[j];
			double ti = wr * a.i[j] + wi * a.r[j];
			a.r[j] = a.r[i] - tr;
			a.i[j] = a.i[i] - ti;
			a.r[i] += tr;
			a.i[i] += ti;
		    }
		}
	    }
	}
   }

	// Log2

	protected double log2(double d)
	{
	    return Math.log(d) / Math.log(2.0);
	}
 
    // Comples
    private class Complex
    {
	double r[];
	double i[];

	private Complex(int l)
	{
	    r = new double[l];
	    i = new double[l];
	}
    }
    // Maximum

    protected class Maxima
    {
	double f[];
	double r[];
	int n[];

	protected Maxima(int l)
	{
	    f = new double[l];
	    r = new double[l];
	    n = new int[l];
	}
    }
}
