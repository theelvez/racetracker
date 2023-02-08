# Import the app module
import app

# Sample data
sample_data = [
    ('Adam Doe', 'Honda Civic', '2020'),
    ('Alice Doe', 'Toyota Camry', '2018'),
    ('Jack Smith', 'Ford Mustang', '2021'),
    ('Julia Smith', 'Volkswagen Golf', '2019'),
    ('Kane Johnson', 'Tesla Model 3', '2022'),
    ('Kelly Johnson', 'Subaru Impreza', '2017'),
    ('Liam Williams', 'Audi A4', '2023'),
    ('Layla Williams', 'Honda Accord', '2020'),
    ('Michael Brown', 'Toyota Corolla', '2018'),
    ('Molly Brown', 'Ford Fiesta', '2021'),
    ('Nathan Smith', 'Volkswagen Jetta', '2019'),
    ('Natalie Smith', 'Tesla Model S', '2022'),
    ('Oliver Johnson', 'Subaru Legacy', '2017'),
    ('Olivia Johnson', 'Audi A6', '2023'),
    ('Paul Miller', 'Honda CR-V', '2020'),
    ('Paige Miller', 'Toyota RAV4', '2018'),
    ('Ryan Robinson', 'Ford Escape', '2021'),
    ('Ruby Robinson', 'Volkswagen Tiguan', '2019'),
    ('Sam Williams', 'Tesla Model X', '2022'),
    ('Sarah Williams', 'Subaru Outback', '2017'),
    ('Tom Brown', 'Audi Q7', '2023'),
    ('Tina Brown', 'Honda Pilot', '2020'),
    ('Uriah Davis', 'Toyota Highlander', '2018'),
    ('Uma Davis', 'Ford Edge', '2021'),
    ('Victor Taylor', 'Volkswagen Atlas', '2019'),
    ('Violet Taylor', 'Tesla Model Y', '2022'),
    ('William Miller', 'Subaru Forester', '2017'),
    ('Wendy Miller', 'Audi Q8', '2023')
]

# Add sample data to database
for data in sample_data:
    app.add_racer(*data)

# View all racers
racers = app.view_all_racers()
print(racers)