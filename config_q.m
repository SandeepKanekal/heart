% Resample to 100 values
% Renormalise to have a max value of 255
% Set values below baseline to the specified baseline
% Export the data as a csv file

function out = config_q(Q_in, outFile)
    % Validate input array
    if ~isvector(Q_in) || isempty(Q_in) || ~isnumeric(Q_in)
        error('Q_in must be a non-empty numeric vector.');
    end

    Q_in = Q_in(:); 

    % Resample to an array of length 100
    nOut = 100;
    xOrig = linspace(0, 1, numel(Q_in));
    xNew  = linspace(0, 1, nOut);
    Q_resampled = interp1(xOrig, Q_in, xNew, 'linear', 'extrap');

    % Handle baseline replacement first (clean up negative artifacting)
    baseline = 5;
    Q_resampled(Q_resampled < baseline) = baseline;

    % Renormalise to have a peak max value of exactly 255
    maxVal = max(Q_resampled);
    minVal = min(Q_resampled);
    
    if maxVal == minVal
        % Avoid division by zero if data is completely flat
        warning('All resampled values are identical. Outputting baseline.');
        Q_scaled = ones(size(Q_resampled)) * baseline;
    else
        Q_scaled = (Q_resampled / maxVal) * 255;
        Q_scaled(Q_scaled < baseline) = baseline;
    end

    Q_scaled = Q_scaled(:);

    % Save as CSV
    try
        writematrix(Q_scaled, outFile);
    catch
        % Fallback for older MATLAB versions
        csvwrite(outFile, Q_scaled);
    end

    % Return the final array to the MATLAB workspace
    out = Q_scaled;
end