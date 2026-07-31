// Clay settings page. Keys must match messageKeys in package.json and the
// enums in src/c/lcars_theme.h.
module.exports = [
  {
    type: 'heading',
    defaultValue: 'LCARS Readout'
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Clock' },
      {
        type: 'select',
        messageKey: 'DATE_FORMAT',
        label: 'Date format',
        defaultValue: '0',
        options: [
          { label: '31.12.2026', value: '0' },
          { label: '12.31.2026', value: '1' },
          { label: '2026-12-31', value: '2' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Weather' },
      {
        type: 'select',
        messageKey: 'TEMP_UNIT',
        label: 'Temperature',
        defaultValue: '0',
        options: [
          { label: 'Celsius', value: '0' },
          { label: 'Fahrenheit', value: '1' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Vibration' },
      {
        type: 'select',
        messageKey: 'VIBE_BT',
        label: 'On lost connection',
        // Short is a 150ms custom pulse; long is the OS's own calibrated
        // short buzz. Anything much under 150ms is hard to feel at all,
        // since the motor spends tens of ms just reaching speed.
        defaultValue: '1',
        options: [
          { label: 'Off', value: '0' },
          { label: 'Short', value: '1' },
          { label: 'Long', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'VIBE_HOURLY',
        label: 'On the hour',
        defaultValue: false
      },
      {
        type: 'text',
        defaultValue: 'Both are skipped while Quiet Time is on.'
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save' }
];
